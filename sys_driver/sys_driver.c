#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/seq_file.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>		/* fops structures */
#include <linux/slab.h>
#include "../convenient.h"

#define MODNAME   "laurijssen"

MODULE_AUTHOR("laurijssen");
MODULE_DESCRIPTION("sysfs char driver");
MODULE_LICENSE("Dual MIT/GPL");
MODULE_VERSION("0.1");

DEFINE_MUTEX(mtx);

#define PLATFORM_DEVICE_NAME "sysfs_laurijssen"

#define SYSFS_FILE1 sysfs_debug_level

static ssize_t sysfs_debug_level_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count);
static ssize_t sysfs_debug_level_show(struct device *dev, struct device_attribute *attr, char *buf);

static int debug_level = 10;

static DEVICE_ATTR_RW(SYSFS_FILE1);

struct platform_device *sysfs_device;

static ssize_t sysfs_debug_level_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	int ret = (int)count, prev_dbglevel;

	if (mutex_lock_interruptible(&mtx))
		return -ERESTARTSYS;

	prev_dbglevel = debug_level;

	if (count == 0 || count > 12)
	{
		ret = -EINVAL;
		goto out;
	}

	ret = kstrtoint(buf, 0, &debug_level);

	if (ret) goto out;

	if (debug_level < 0 || debug_level > 12) {
		pr_info("trying to set invalid value (%d) for debug_level\n"
			" [allowed range: %d-%d]; resetting to previous (%d)\n",
			debug_level, 0, 12, prev_dbglevel);
		debug_level = prev_dbglevel;
		ret = -EFAULT;
		goto out;
	}

	ret = count;

out:
	mutex_unlock(&mtx);
	return ret;
}

static ssize_t sysfs_debug_level_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int n;

	if (mutex_lock_interruptible(&mtx))
		return -ERESTARTSYS;

	pr_debug("in the 'show' method: name %s debug_level %d\n", dev->kobj.name, debug_level);

	n = snprintf(buf, 25, "%d", debug_level);

	mutex_unlock(&mtx);

	return n;
}

static int __init sysdrv_init(void)
{
	sysfs_device = platform_device_register_simple(PLATFORM_DEVICE_NAME, -1, NULL, 0);
	device_create_file(&sysfs_device->dev, &dev_attr_SYSFS_FILE1);
	return 0;		/* success */
}

static void __exit sysdrv_exit(void)
{
	platform_device_unregister(sysfs_device);
}

module_init(sysdrv_init);
module_exit(sysdrv_exit);
