#include <linux/module.h>           // basic kernel module definitions
#include <linux/platform_device.h>  // platform driver/device definitions
#include <linux/mod_devicetable.h>  // of_device_id, MODULE_DEVICE_TABLE
#include <linux/io.h>               // iowrite32/ioread32 functions
#include <linux/mutex.h>            // mutex defintions
#include <linux/miscdevice.h>       // miscdevice defintions
#include <linux/types.h>            // data types like u32, u16, etc.
#include <linux/fs.h>               // copy_to_user, etc.
#include <linux/kstrtox.h>          // kstrtou8, etc.

#define DUTY_RED_OFFSET 0x0
#define DUTY_GREEN_OFFSET 0x4
#define DUTY_BLUE_OFFSET 0x8
#define BASE_PERIOD_OFFSET 0x12

#define SPAN 16

/**
* struct pwm_rgb_dev - Private rgb pwm controller device struct.
* @base_addr: Pointer to the component's base address
* @duty_red: Address of the red duty cycle register
* @duty_green: Address of the green duty cycle register
* @duty_blue: Address of the blue duty cycle register
* @base_period: Address of the pwm base period register
* @miscdev: miscdevice used to create a character device
* @lock: mutex used to prevent concurrent writes to memory
*
* An pwm_rgb_dev struct gets created for each led patterns component.
*/
struct pwm_rgb_dev {
void __iomem *base_addr;
void __iomem *duty_red;
void __iomem *duty_green;
void __iomem *duty_blue;
void __iomem *base_period;
struct miscdevice miscdev;
struct mutex lock;
};

/**
* duty_red_show() - Return the duty_red value
* to user-space via sysfs.
* @dev: Device structure for the pwm_rgb_controller component. This
* device struct is embedded in the pwm_rgb_controller' device struct.
* @attr: Unused.
* @buf: Buffer that gets returned to user-space.
*
* Return: The number of bytes read.
*/
static ssize_t duty_red_show(struct device *dev,
struct device_attribute *attr, char *buf)
{
u32 duty_red;
struct pwm_rgb_dev *priv = dev_get_drvdata(dev);

duty_red = ioread32(priv->duty_red);

return scnprintf(buf, PAGE_SIZE, "%u\n", duty_red);
}

/**
* duty_red_store() - Store the duty_red value.
* @dev: Device structure for the pwm_rgb_controller component. This
* device struct is embedded in the pwm_rgb_controller'
* platform device struct.
* @attr: Unused.
* @buf: Buffer that contains the duty_red value being written.
* @size: The number of bytes being written.
*
* Return: The number of bytes stored.
*/
static ssize_t duty_red_store(struct device *dev,
struct device_attribute *attr, const char *buf, size_t size)
{
u32 duty_red;
int ret;
struct pwm_rgb_dev *priv = dev_get_drvdata(dev);

// Parse the string we received as an unsigned 32-bit int
// See https://elixir.bootlin.com/linux/latest/source/lib/kstrtox.c#L289
ret = kstrtouint(buf, 0, &duty_red);
if (ret < 0) {
// kstrtobool returned an error
return ret;
}

iowrite32(duty_red, priv->duty_red);

// Write was successful, so we return the number of bytes we wrote.
return size;
}

/**
* duty_green_show() - Return the duty_green value
* to user-space via sysfs.
* @dev: Device structure for the pwm_rgb_controller component. This
* device struct is embedded in the pwm_rgb_controller' device struct.
* @attr: Unused.
* @buf: Buffer that gets returned to user-space.
*
* Return: The number of bytes read.
*/
static ssize_t duty_green_show(struct device *dev,
struct device_attribute *attr, char *buf)
{
u32 duty_green;
struct pwm_rgb_dev *priv = dev_get_drvdata(dev);

duty_green = ioread32(priv->duty_green);

return scnprintf(buf, PAGE_SIZE, "%u\n", duty_green);
}

/**
* duty_green_store() - Store the duty_green value.
* @dev: Device structure for the pwm_rgb_controller component. This
* device struct is embedded in the pwm_rgb_controller'
* platform device struct.
* @attr: Unused.
* @buf: Buffer that contains the duty_green value being written.
* @size: The number of bytes being written.
*
* Return: The number of bytes stored.
*/
static ssize_t duty_green_store(struct device *dev,
struct device_attribute *attr, const char *buf, size_t size)
{
u32 duty_green;
int ret;
struct pwm_rgb_dev *priv = dev_get_drvdata(dev);

// Parse the string we received as an unsigned 32-bit int
// See https://elixir.bootlin.com/linux/latest/source/lib/kstrtox.c#L289
ret = kstrtouint(buf, 0, &duty_green);
if (ret < 0) {
// kstrtobool returned an error
return ret;
}

iowrite32(duty_green, priv->duty_green);

// Write was successful, so we return the number of bytes we wrote.
return size;
}

/**
* duty_blue_show() - Return the duty_blue value
* to user-space via sysfs.
* @dev: Device structure for the pwm_rgb_controller component. This
* device struct is embedded in the pwm_rgb_controller' device struct.
* @attr: Unused.
* @buf: Buffer that gets returned to user-space.
*
* Return: The number of bytes read.
*/
static ssize_t duty_blue_show(struct device *dev,
struct device_attribute *attr, char *buf)
{
u32 duty_blue;
struct pwm_rgb_dev *priv = dev_get_drvdata(dev);

duty_blue = ioread32(priv->duty_blue);

return scnprintf(buf, PAGE_SIZE, "%u\n", duty_blue);
}

/**
* duty_blue_store() - Store the duty_blue value.
* @dev: Device structure for the pwm_rgb_controller component. This
* device struct is embedded in the pwm_rgb_controller'
* platform device struct.
* @attr: Unused.
* @buf: Buffer that contains the duty_blue value being written.
* @size: The number of bytes being written.
*
* Return: The number of bytes stored.
*/
static ssize_t duty_blue_store(struct device *dev,
struct device_attribute *attr, const char *buf, size_t size)
{
u32 duty_blue;
int ret;
struct pwm_rgb_dev *priv = dev_get_drvdata(dev);

// Parse the string we received as an unsigned 32-bit int
// See https://elixir.bootlin.com/linux/latest/source/lib/kstrtox.c#L289
ret = kstrtouint(buf, 0, &duty_blue);
if (ret < 0) {
// kstrtobool returned an error
return ret;
}

iowrite32(duty_blue, priv->duty_blue);

// Write was successful, so we return the number of bytes we wrote.
return size;
}

/**
* base_period_show() - Return the base_period value
* to user-space via sysfs.
* @dev: Device structure for the pwm_rgb_controller component. This
* device struct is embedded in the pwm_rgb_controller' device struct.
* @attr: Unused.
* @buf: Buffer that gets returned to user-space.
*
* Return: The number of bytes read.
*/
static ssize_t base_period_show(struct device *dev,
struct device_attribute *attr, char *buf)
{
u32 base_period;
struct pwm_rgb_dev *priv = dev_get_drvdata(dev);

base_period = ioread32(priv->base_period);

return scnprintf(buf, PAGE_SIZE, "%u\n", base_period);
}

/**
* base_period_store() - Store the base_period value.
* @dev: Device structure for the pwm_rgb_controller component. This
* device struct is embedded in the pwm_rgb_controller'
* platform device struct.
* @attr: Unused.
* @buf: Buffer that contains the base_period value being written.
* @size: The number of bytes being written.
*
* Return: The number of bytes stored.
*/
static ssize_t base_period_store(struct device *dev,
struct device_attribute *attr, const char *buf, size_t size)
{
u32 base_period;
int ret;
struct pwm_rgb_dev *priv = dev_get_drvdata(dev);

// Parse the string we received as an unsigned 32-bit int
// See https://elixir.bootlin.com/linux/latest/source/lib/kstrtox.c#L289
ret = kstrtouint(buf, 0, &base_period);
if (ret < 0) {
// kstrtobool returned an error
return ret;
}

iowrite32(base_period, priv->base_period);

// Write was successful, so we return the number of bytes we wrote.
return size;
}

// Define sysfs attributes
static DEVICE_ATTR_RW(duty_red);
static DEVICE_ATTR_RW(duty_green);
static DEVICE_ATTR_RW(duty_blue);
static DEVICE_ATTR_RW(base_period);

// Create an attribute group so the device core can
// export the attributes for us.
static struct attribute *pwm_rgb_attrs[] = {
&dev_attr_duty_red.attr,
&dev_attr_duty_green.attr,
&dev_attr_duty_blue.attr,
&dev_attr_base_period.attr,
NULL,
};
ATTRIBUTE_GROUPS(pwm_rgb);

/**
* pwm_rgb_read() - Read method for the pwm_rgb char device
* @file: Pointer to the char device file struct.
* @buf: User-space buffer to read the value into.
* @count: The number of bytes being requested.
* @offset: The byte offset in the file being read from.
*
* Return: On success, the number of bytes written is returned and the
* offset @offset is advanced by this number. On error, a negative error
* value is returned.
*/
static ssize_t pwm_rgb_read(struct file *file, char __user *buf,
size_t count, loff_t *offset)
{
size_t ret;
u32 val;

/*
* Get the device's private data from the file struct's private_data
* field. The private_data field is equal to the miscdev field in the
* pwm_rgb_dev struct. container_of returns the
* pwm_rgb_dev struct that contains the miscdev in private_data.
*/
struct pwm_rgb_dev *priv = container_of(file->private_data,
struct pwm_rgb_dev, miscdev);

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
pr_warn("pwm_rgb_read: unaligned access\n");
return -EFAULT;
}

val = ioread32(priv->base_addr + *offset);

// Copy the value to userspace.
ret = copy_to_user(buf, &val, sizeof(val));
if (ret == sizeof(val)) {
pr_warn("pwm_rgb_read: nothing copied\n");
return -EFAULT;
}

// Increment the file offset by the number of bytes we read.
*offset = *offset + sizeof(val);

return sizeof(val);
}

/**
* pwm_rgb_write() - Write method for the pwm_rgb char device
* @file: Pointer to the char device file struct.
* @buf: User-space buffer to read the value from.
* @count: The number of bytes being written.
* @offset: The byte offset in the file being written to.
*
* Return: On success, the number of bytes written is returned and the
* offset @offset is advanced by this number. On error, a negative error
* value is returned.
*/
static ssize_t pwm_rgb_write(struct file *file, const char __user *buf,
size_t count, loff_t *offset)
{
size_t ret;
u32 val;

struct pwm_rgb_dev *priv = container_of(file->private_data,
struct pwm_rgb_dev, miscdev);

if (*offset < 0) {
return -EINVAL;
}
if (*offset >= SPAN) {
return 0;
}
if ((*offset % 0x4) != 0) {
pr_warn("pwm_rgb_write: unaligned access\n");
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
pr_warn("pwm_rgb_write: nothing copied from user space\n");
ret = -EFAULT;
}

mutex_unlock(&priv->lock);
return ret;
}

/**
* pwm_rgb_fops - File operations supported by the
* pwm_rgb driver
* @owner: The pwm_rgb driver owns the file operations; this
* ensures that the driver can't be removed while the
* character device is still in use.
* @read: The read function.
* @write: The write function.
* @llseek: We use the kernel's default_llseek() function; this allows
* users to change what position they are writing/reading to/from.
*/
static const struct file_operations pwm_rgb_fops = {
.owner = THIS_MODULE,
.read = pwm_rgb_read,
.write = pwm_rgb_write,
.llseek = default_llseek,
};

static int pwm_rgb_probe(struct platform_device *pdev)
{

size_t ret;

struct pwm_rgb_dev *priv;
/*
* Allocate kernel memory for the led patterns device and set it to 0.
* GFP_KERNEL specifies that we are allocating normal kernel RAM;
* see the kmalloc documentation for more info. The allocated memory
* is automatically freed when the device is removed.
*/
priv = devm_kzalloc(&pdev->dev, sizeof(struct pwm_rgb_dev),
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
priv->duty_red = priv->base_addr + DUTY_RED_OFFSET;
priv->duty_green = priv->base_addr + DUTY_GREEN_OFFSET;
priv->duty_blue = priv->base_addr + DUTY_BLUE_OFFSET;
priv->base_period = priv->base_addr + BASE_PERIOD_OFFSET;
// turn on red, just for fun.
iowrite32(0xffff, priv->duty_red);
iowrite32(0x0, priv->duty_green);
iowrite32(0x0, priv->duty_blue);
// set period to 1 ms
iowrite32(0x1000, priv->base_period);

// Initialize the misc device parameters
priv->miscdev.minor = MISC_DYNAMIC_MINOR;
priv->miscdev.name = "pwm_rgb";
priv->miscdev.fops = &pwm_rgb_fops;
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
pr_info("pwm_rgb_probe successful\n");
return 0;
}

/**
* pwm_rgb_remove() - Remove an pwm_rgb device.
* @pdev: Platform device structure associated with our pwm_rgb device.
*
* This function is called when an pwm_rgb devicee is removed or
* the driver is removed.
*/
static int pwm_rgb_remove(struct platform_device *pdev)
{
// Get the pwm_rgb's private data from the platform device.
struct pwm_rgb_dev *priv = platform_get_drvdata(pdev);
// Turn off LED for kicks.
iowrite32(0x0, priv->duty_red);
iowrite32(0x0, priv->duty_green);
iowrite32(0x0, priv->duty_blue);

// Deregister the misc device and remove the /dev/led_patterns file.
misc_deregister(&priv->miscdev);
pr_info("pwm_rgb_remove successful\n");

return 0;
}


/*
* Define the compatible property used for matching devices to this driver,
* then add our device id structure to the kernel's device table. For a device
* to be matched with this driver, its device tree node must use the same
* compatible string as defined here.
*/
static const struct of_device_id pwm_rgb_of_match[] = {
    { .compatible = "jensen,pwm_rgb", },
    { }
};
MODULE_DEVICE_TABLE(of, pwm_rgb_of_match);

/*
* struct pwm_rgb_driver - Platform driver struct for the pwm_rgb driver
* @probe: Function that's called when a device is found
* @remove: Function that's called when a device is removed
* @driver.owner: Which module owns this driver
* @driver.name: Name of the pwm_rgb driver
* @driver.of_match_table: Device tree match table
*/
static struct platform_driver pwm_rgb_driver = {
    .probe = pwm_rgb_probe,
    .remove = pwm_rgb_remove,
    .driver = {
        .owner = THIS_MODULE,
        .name = "pwm_rgb",
        .of_match_table = pwm_rgb_of_match,
        .dev_groups = pwm_rgb_groups,
    },
};

/*
* We don't need to do anything special in module init/exit.
* This macro automatically handles module init/exit.
*/
module_platform_driver(pwm_rgb_driver);

MODULE_LICENSE("Dual MIT/GPL");
MODULE_AUTHOR("David Jensen");
MODULE_DESCRIPTION("pwm_rgb driver");