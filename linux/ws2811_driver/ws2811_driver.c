#include <linux/module.h>           // basic kernel module definitions
#include <linux/platform_device.h>  // platform driver/device definitions
#include <linux/mod_devicetable.h>  // of_device_id, MODULE_DEVICE_TABLE
#include <linux/io.h>               // iowrite32/ioread32 functions
#include <linux/mutex.h>            // mutex defintions
#include <linux/miscdevice.h>       // miscdevice defintions
#include <linux/types.h>            // data types like u32, u16, etc.
#include <linux/fs.h>               // copy_to_user, etc.
#include <linux/kstrtox.h>          // kstrtou8, etc.

#define RGB_ALL 0x0
#define RGB_SINGLE 0x4
#define STRIP_INDEX 0x8

#define SPAN 12

/**
* struct ws2811_dev - Private rgb pwm controller device struct.
* @base_addr: Pointer to the component's base address
* @rgb_all: Address of the red duty cycle register
* @rgb_single: Address of the green duty cycle register
* @strip_index: Address of the blue duty cycle register
* @miscdev: miscdevice used to create a character device
* @lock: mutex used to prevent concurrent writes to memory
*
* An ws2811_dev struct gets created for each led patterns component.
*/
struct ws2811_dev {
void __iomem *base_addr;
void __iomem *rgb_all;
void __iomem *rgb_single;
void __iomem *strip_index;
struct miscdevice miscdev;
struct mutex lock;
};

/**
* rgb_all_show() - Return the rgb_all value
* to user-space via sysfs.
* @dev: Device structure for the ws2811_controller component. This
* device struct is embedded in the ws2811_controller' device struct.
* @attr: Unused.
* @buf: Buffer that gets returned to user-space.
*
* Return: The number of bytes read.
*/
static ssize_t rgb_all_show(struct device *dev,
struct device_attribute *attr, char *buf)
{
u32 rgb_all;
struct ws2811_dev *priv = dev_get_drvdata(dev);

rgb_all = ioread32(priv->rgb_all);

return scnprintf(buf, PAGE_SIZE, "%u\n", rgb_all);
}

/**
* rgb_all_store() - Store the rgb_all value.
* @dev: Device structure for the ws2811_controller component. This
* device struct is embedded in the ws2811_controller'
* platform device struct.
* @attr: Unused.
* @buf: Buffer that contains the rgb_all value being written.
* @size: The number of bytes being written.
*
* Return: The number of bytes stored.
*/
static ssize_t rgb_all_store(struct device *dev,
struct device_attribute *attr, const char *buf, size_t size)
{
u32 rgb_all;
int ret;
struct ws2811_dev *priv = dev_get_drvdata(dev);

// Parse the string we received as an unsigned 32-bit int
// See https://elixir.bootlin.com/linux/latest/source/lib/kstrtox.c#L289
ret = kstrtouint(buf, 0, &rgb_all);
if (ret < 0) {
// kstrtobool returned an error
return ret;
}

iowrite32(rgb_all, priv->rgb_all);

// Write was successful, so we return the number of bytes we wrote.
return size;
}

/**
* rgb_single_show() - Return the rgb_single value
* to user-space via sysfs.
* @dev: Device structure for the ws2811_controller component. This
* device struct is embedded in the ws2811_controller' device struct.
* @attr: Unused.
* @buf: Buffer that gets returned to user-space.
*
* Return: The number of bytes read.
*/
static ssize_t rgb_single_show(struct device *dev,
struct device_attribute *attr, char *buf)
{
u32 rgb_single;
struct ws2811_dev *priv = dev_get_drvdata(dev);

rgb_single = ioread32(priv->rgb_single);

return scnprintf(buf, PAGE_SIZE, "%u\n", rgb_single);
}

/**
* rgb_single_store() - Store the rgb_single value.
* @dev: Device structure for the ws2811_controller component. This
* device struct is embedded in the ws2811_controller'
* platform device struct.
* @attr: Unused.
* @buf: Buffer that contains the rgb_single value being written.
* @size: The number of bytes being written.
*
* Return: The number of bytes stored.
*/
static ssize_t rgb_single_store(struct device *dev,
struct device_attribute *attr, const char *buf, size_t size)
{
u32 rgb_single;
int ret;
struct ws2811_dev *priv = dev_get_drvdata(dev);

// Parse the string we received as an unsigned 32-bit int
// See https://elixir.bootlin.com/linux/latest/source/lib/kstrtox.c#L289
ret = kstrtouint(buf, 0, &rgb_single);
if (ret < 0) {
// kstrtobool returned an error
return ret;
}

iowrite32(rgb_single, priv->rgb_single);

// Write was successful, so we return the number of bytes we wrote.
return size;
}

/**
* strip_index_show() - Return the strip_index value
* to user-space via sysfs.
* @dev: Device structure for the ws2811_controller component. This
* device struct is embedded in the ws2811_controller' device struct.
* @attr: Unused.
* @buf: Buffer that gets returned to user-space.
*
* Return: The number of bytes read.
*/
static ssize_t strip_index_show(struct device *dev,
struct device_attribute *attr, char *buf)
{
u32 strip_index;
struct ws2811_dev *priv = dev_get_drvdata(dev);

strip_index = ioread32(priv->strip_index);

return scnprintf(buf, PAGE_SIZE, "%u\n", strip_index);
}

/**
* strip_index_store() - Store the strip_index value.
* @dev: Device structure for the ws2811_controller component. This
* device struct is embedded in the ws2811_controller'
* platform device struct.
* @attr: Unused.
* @buf: Buffer that contains the strip_index value being written.
* @size: The number of bytes being written.
*
* Return: The number of bytes stored.
*/
static ssize_t strip_index_store(struct device *dev,
struct device_attribute *attr, const char *buf, size_t size)
{
u32 strip_index;
int ret;
struct ws2811_dev *priv = dev_get_drvdata(dev);

// Parse the string we received as an unsigned 32-bit int
// See https://elixir.bootlin.com/linux/latest/source/lib/kstrtox.c#L289
ret = kstrtouint(buf, 0, &strip_index);
if (ret < 0) {
// kstrtobool returned an error
return ret;
}

iowrite32(strip_index, priv->strip_index);

// Write was successful, so we return the number of bytes we wrote.
return size;
}

// Define sysfs attributes
static DEVICE_ATTR_RW(rgb_all);
static DEVICE_ATTR_RW(rgb_single);
static DEVICE_ATTR_RW(strip_index);

// Create an attribute group so the device core can
// export the attributes for us.
static struct attribute *ws2811_attrs[] = {
&dev_attr_rgb_all.attr,
&dev_attr_rgb_single.attr,
&dev_attr_strip_index.attr,
NULL,
};
ATTRIBUTE_GROUPS(ws2811);

/**
* ws2811_read() - Read method for the ws2811 char device
* @file: Pointer to the char device file struct.
* @buf: User-space buffer to read the value into.
* @count: The number of bytes being requested.
* @offset: The byte offset in the file being read from.
*
* Return: On success, the number of bytes written is returned and the
* offset @offset is advanced by this number. On error, a negative error
* value is returned.
*/
static ssize_t ws2811_read(struct file *file, char __user *buf,
size_t count, loff_t *offset)
{
size_t ret;
u32 val;

/*
* Get the device's private data from the file struct's private_data
* field. The private_data field is equal to the miscdev field in the
* ws2811_dev struct. container_of returns the
* ws2811_dev struct that contains the miscdev in private_data.
*/
struct ws2811_dev *priv = container_of(file->private_data,
struct ws2811_dev, miscdev);

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
pr_warn("ws2811_read: unaligned access\n");
return -EFAULT;
}

val = ioread32(priv->base_addr + *offset);

// Copy the value to userspace.
ret = copy_to_user(buf, &val, sizeof(val));
if (ret == sizeof(val)) {
pr_warn("ws2811_read: nothing copied\n");
return -EFAULT;
}

// Increment the file offset by the number of bytes we read.
*offset = *offset + sizeof(val);

return sizeof(val);
}

/**
* ws2811_write() - Write method for the ws2811 char device
* @file: Pointer to the char device file struct.
* @buf: User-space buffer to read the value from.
* @count: The number of bytes being written.
* @offset: The byte offset in the file being written to.
*
* Return: On success, the number of bytes written is returned and the
* offset @offset is advanced by this number. On error, a negative error
* value is returned.
*/
static ssize_t ws2811_write(struct file *file, const char __user *buf,
size_t count, loff_t *offset)
{
size_t ret;
u32 val;

struct ws2811_dev *priv = container_of(file->private_data,
struct ws2811_dev, miscdev);

if (*offset < 0) {
return -EINVAL;
}
if (*offset >= SPAN) {
return 0;
}
if ((*offset % 0x4) != 0) {
pr_warn("ws2811_write: unaligned access\n");
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
pr_warn("ws2811_write: nothing copied from user space\n");
ret = -EFAULT;
}

mutex_unlock(&priv->lock);
return ret;
}

/**
* ws2811_fops - File operations supported by the
* ws2811 driver
* @owner: The ws2811 driver owns the file operations; this
* ensures that the driver can't be removed while the
* character device is still in use.
* @read: The read function.
* @write: The write function.
* @llseek: We use the kernel's default_llseek() function; this allows
* users to change what position they are writing/reading to/from.
*/
static const struct file_operations ws2811_fops = {
.owner = THIS_MODULE,
.read = ws2811_read,
.write = ws2811_write,
.llseek = default_llseek,
};

static int ws2811_probe(struct platform_device *pdev)
{

size_t ret;

struct ws2811_dev *priv;
/*
* Allocate kernel memory for the led patterns device and set it to 0.
* GFP_KERNEL specifies that we are allocating normal kernel RAM;
* see the kmalloc documentation for more info. The allocated memory
* is automatically freed when the device is removed.
*/
priv = devm_kzalloc(&pdev->dev, sizeof(struct ws2811_dev),
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
priv->rgb_all = priv->base_addr + RGB_ALL;
priv->rgb_single = priv->base_addr + RGB_SINGLE;
priv->strip_index = priv->base_addr + STRIP_INDEX;
// turn on red, just for fun.
iowrite32(0xffff, priv->rgb_all);
iowrite32(0x0, priv->rgb_single);
iowrite32(0x0, priv->strip_index);

// Initialize the misc device parameters
priv->miscdev.minor = MISC_DYNAMIC_MINOR;
priv->miscdev.name = "ws2811";
priv->miscdev.fops = &ws2811_fops;
priv->miscdev.parent = &pdev->dev;

// Register the misc device; this creates a char dev at /dev/ws2811
ret = misc_register(&priv->miscdev);
if (ret) {
pr_err("Failed to register misc device");
return ret;
}

/* Attach the ws2811's private data to the platform device's struct.
* This is so we can access our state container in the other functions.
*/
platform_set_drvdata(pdev, priv);
pr_info("ws2811_probe successful\n");
return 0;
}

/**
* ws2811_remove() - Remove an ws2811 device.
* @pdev: Platform device structure associated with our ws2811 device.
*
* This function is called when an ws2811 devicee is removed or
* the driver is removed.
*/
static int ws2811_remove(struct platform_device *pdev)
{
// Get the ws2811's private data from the platform device.
struct ws2811_dev *priv = platform_get_drvdata(pdev);
// Turn off LED for kicks.
iowrite32(0x0, priv->rgb_all);
iowrite32(0x0, priv->rgb_single);
iowrite32(0x0, priv->strip_index);

// Deregister the misc device and remove the /dev/led_patterns file.
misc_deregister(&priv->miscdev);
pr_info("ws2811_remove successful\n");

return 0;
}


/*
* Define the compatible property used for matching devices to this driver,
* then add our device id structure to the kernel's device table. For a device
* to be matched with this driver, its device tree node must use the same
* compatible string as defined here.
*/
static const struct of_device_id ws2811_of_match[] = {
    { .compatible = "jensen,ws2811", },
    { }
};
MODULE_DEVICE_TABLE(of, ws2811_of_match);

/*
* struct ws2811_driver - Platform driver struct for the ws2811 driver
* @probe: Function that's called when a device is found
* @remove: Function that's called when a device is removed
* @driver.owner: Which module owns this driver
* @driver.name: Name of the ws2811 driver
* @driver.of_match_table: Device tree match table
*/
static struct platform_driver ws2811_driver = {
    .probe = ws2811_probe,
    .remove = ws2811_remove,
    .driver = {
        .owner = THIS_MODULE,
        .name = "ws2811",
        .of_match_table = ws2811_of_match,
        .dev_groups = ws2811_groups,
    },
};

/*
* We don't need to do anything special in module init/exit.
* This macro automatically handles module init/exit.
*/
module_platform_driver(ws2811_driver);

MODULE_LICENSE("Dual MIT/GPL");
MODULE_AUTHOR("Peter Buckley");
MODULE_DESCRIPTION("ws2811 driver");
