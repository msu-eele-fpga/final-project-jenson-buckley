#include <linux/module.h>           // basic kernel module definitions
#include <linux/platform_device.h>  // platform driver/device definitions
#include <linux/mod_devicetable.h>  // of_device_id, MODULE_DEVICE_TABLE
#include <linux/io.h>               // iowrite32/ioread32 functions
#include <linux/mutex.h>            // mutex defintions
#include <linux/miscdevice.h>       // miscdevice defintions
#include <linux/types.h>            // data types like u32, u16, etc.
#include <linux/fs.h>               // copy_to_user, etc.
#include <linux/kstrtox.h>          // kstrtou8, etc.

#define STOP_BUTTON_OFFSET 0x0

#define SPAN 16

/**
* struct stop_button_dev - Private stop button device struct.
* @base_addr: Pointer to the component's base address
* @stop_button: Address of the stop button register
* @miscdev: miscdevice used to create a character device
* @lock: mutex used to prevent concurrent writes to memory
*
* An stop_button_dev struct gets created for each led patterns component.
*/
struct stop_button_dev {
void __iomem *base_addr;
void __iomem *stop_button;
struct miscdevice miscdev;
struct mutex lock;
};

/**
* stop_button_show() - Return the stop_button value
* to user-space via sysfs.
* @dev: Device structure for the stop_button_controller component. This
* device struct is embedded in the stop_button_controller' device struct.
* @attr: Unused.
* @buf: Buffer that gets returned to user-space.
*
* Return: The number of bytes read.
*/
static ssize_t stop_button_show(struct device *dev,
struct device_attribute *attr, char *buf)
{
u32 stop_button;
struct stop_button_dev *priv = dev_get_drvdata(dev);

stop_button = ioread32(priv->stop_button);

return scnprintf(buf, PAGE_SIZE, "%u\n", stop_button);
}

/**
* stop_button_store() - Store the stop_button value.
* @dev: Device structure for the stop_button_controller component. This
* device struct is embedded in the stop_button_controller
* platform device struct.
* @attr: Unused.
* @buf: Buffer that contains the stop_button value being written.
* @size: The number of bytes being written.
*
* Return: The number of bytes stored.
*/
static ssize_t stop_button_store(struct device *dev,
struct device_attribute *attr, const char *buf, size_t size)
{
u32 stop_button;
int ret;
struct stop_button_dev *priv = dev_get_drvdata(dev);

// Parse the string we received as an unsigned 32-bit int
// See https://elixir.bootlin.com/linux/latest/source/lib/kstrtox.c#L289
ret = kstrtouint(buf, 0, &stop_button);
if (ret < 0) {
// kstrtobool returned an error
return ret;
}

iowrite32(stop_button, priv->stop_button);

// Write was successful, so we return the number of bytes we wrote.
return size;
}

// Define sysfs attributes
static DEVICE_ATTR_RW(stop_button);

// Create an attribute group so the device core can
// export the attributes for us.
static struct attribute *stop_button_attrs[] = {
&dev_attr_stop_button.attr,
NULL,
};
ATTRIBUTE_GROUPS(stop_button);

/**
* stop_button_read() - Read method for the stop_button char device
* @file: Pointer to the char device file struct.
* @buf: User-space buffer to read the value into.
* @count: The number of bytes being requested.
* @offset: The byte offset in the file being read from.
*
* Return: On success, the number of bytes written is returned and the
* offset @offset is advanced by this number. On error, a negative error
* value is returned.
*/
static ssize_t stop_button_read(struct file *file, char __user *buf,
size_t count, loff_t *offset)
{
size_t ret;
u32 val;

/*
* Get the device's private data from the file struct's private_data
* field. The private_data field is equal to the miscdev field in the
* stop_button_dev struct. container_of returns the
* stop_button_dev struct that contains the miscdev in private_data.
*/
struct stop_button_dev *priv = container_of(file->private_data,
struct stop_button_dev, miscdev);

// Check file offset to make sure we are reading from a valid location.
if (*offset < 0) {
// We can't read from a negative file position.
return -EINVAL;
}
if (*offset >= SPAN) {
// We can't read from a position past the end of our device.
return 0;
}
if ((*offset % 0x4) != 0) {
// Prevent unaligned access.
pr_warn("stop_button_read: unaligned access\n");
return -EFAULT;
}

val = ioread32(priv->base_addr + *offset);

// Copy the value to userspace.
ret = copy_to_user(buf, &val, sizeof(val));
if (ret == sizeof(val)) {
pr_warn("stop_button_read: nothing copied\n");
return -EFAULT;
}

// Increment the file offset by the number of bytes we read.
*offset = *offset + sizeof(val);

return sizeof(val);
}

/**
* stop_button_write() - Write method for the stop_button char device
* @file: Pointer to the char device file struct.
* @buf: User-space buffer to read the value from.
* @count: The number of bytes being written.
* @offset: The byte offset in the file being written to.
*
* Return: On success, the number of bytes written is returned and the
* offset @offset is advanced by this number. On error, a negative error
* value is returned.
*/
static ssize_t stop_button_write(struct file *file, const char __user *buf,
size_t count, loff_t *offset)
{
size_t ret;
u32 val;

struct stop_button_dev *priv = container_of(file->private_data,
struct stop_button_dev, miscdev);

if (*offset < 0) {
return -EINVAL;
}
if (*offset >= SPAN) {
return 0;
}
if ((*offset % 0x4) != 0) {
pr_warn("stop_button_write: unaligned access\n");
return -EFAULT;
}

mutex_lock(&priv->lock);

// Get the value from userspace.
ret = copy_from_user(&val, buf, sizeof(val));
if (ret != sizeof(val)) {
iowrite32(val, priv->base_addr + *offset);

// Increment the file offset by the number of bytes we wrote.
*offset = *offset + sizeof(val);

// Return the number of bytes we wrote.
ret = sizeof(val);
}
else {
pr_warn("stop_button_write: nothing copied from user space\n");
ret = -EFAULT;
}

mutex_unlock(&priv->lock);
return ret;
}

/**
* stop_button_fops - File operations supported by the
* stop_button driver
* @owner: The stop_button driver owns the file operations; this
* ensures that the driver can't be removed while the
* character device is still in use.
* @read: The read function.
* @write: The write function.
* @llseek: We use the kernel's default_llseek() function; this allows
* users to change what position they are writing/reading to/from.
*/
static const struct file_operations stop_button_fops = {
.owner = THIS_MODULE,
.read = stop_button_read,
.write = stop_button_write,
.llseek = default_llseek,
};

static int stop_button_probe(struct platform_device *pdev)
{

size_t ret;

struct stop_button_dev *priv;
/*
* Allocate kernel memory for the led patterns device and set it to 0.
* GFP_KERNEL specifies that we are allocating normal kernel RAM;
* see the kmalloc documentation for more info. The allocated memory
* is automatically freed when the device is removed.
*/
priv = devm_kzalloc(&pdev->dev, sizeof(struct stop_button_dev),
GFP_KERNEL);
if (!priv) {
pr_err("Failed to allocate memory\n");
return -ENOMEM;
}
/*
* Request and remap the device's memory region. Requesting the region
* make sure nobody else can use that memory. The memory is remapped
* into the kernel's virtual address space because we don't have access
* to physical memory locations.
*/
priv->base_addr = devm_platform_ioremap_resource(pdev, 0);
if (IS_ERR(priv->base_addr)) {
pr_err("Failed to request/remap platform device resource\n");
return PTR_ERR(priv->base_addr);
}
// Set the memory addresses for each register.
priv->stop_button = priv->base_addr + STOP_BUTTON_OFFSET;
// force button to low
iowrite32(0x0, priv->duty_red);

// Initialize the misc device parameters
priv->miscdev.minor = MISC_DYNAMIC_MINOR;
priv->miscdev.name = "stop_button";
priv->miscdev.fops = &stop_button_fops;
priv->miscdev.parent = &pdev->dev;

// Register the misc device; this creates a char dev at /dev/pwm_rgb
ret = misc_register(&priv->miscdev);
if (ret) {
pr_err("Failed to register misc device");
return ret;
}

/* Attach the pwm_rgb's private data to the platform device's struct.
* This is so we can access our state container in the other functions.
*/
platform_set_drvdata(pdev, priv);
pr_info("stop_button_probe successful\n");
return 0;
}

/**
* stop_button_remove() - Remove an stop_button device.
* @pdev: Platform device structure associated with our stop_button device.
*
* This function is called when an stop_button devicee is removed or
* the driver is removed.
*/
static int stop_button_remove(struct platform_device *pdev)
{
// Get the stop_button's private data from the platform device.
struct stop_button_dev *priv = platform_get_drvdata(pdev);
// Force button low
iowrite32(0x0, priv->stop_button);

// Deregister the misc device and remove the /dev/led_patterns file.
misc_deregister(&priv->miscdev);
pr_info("stop_button_remove successful\n");

return 0;
}


/*
* Define the compatible property used for matching devices to this driver,
* then add our device id structure to the kernel's device table. For a device
* to be matched with this driver, its device tree node must use the same
* compatible string as defined here.
*/
static const struct of_device_id stop_button_of_match[] = {
    { .compatible = "jensen,stop_button", },
    { }
};
MODULE_DEVICE_TABLE(of, stop_button_of_match);

/*
* struct stop_button_driver - Platform driver struct for the stop_button driver
* @probe: Function that's called when a device is found
* @remove: Function that's called when a device is removed
* @driver.owner: Which module owns this driver
* @driver.name: Name of the pwm_rgb driver
* @driver.of_match_table: Device tree match table
*/
static struct platform_driver stop_button_driver = {
    .probe = stop_button_probe,
    .remove = stop_button_remove,
    .driver = {
        .owner = THIS_MODULE,
        .name = "stop_button",
        .of_match_table = stop_button_of_match,
        .dev_groups = stop_button_groups,
    },
};

/*
* We don't need to do anything special in module init/exit.
* This macro automatically handles module init/exit.
*/
module_platform_driver(stop_button_driver);

MODULE_LICENSE("Dual MIT/GPL");
MODULE_AUTHOR("David Jensen");
MODULE_DESCRIPTION("stop button driver");