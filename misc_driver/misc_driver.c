#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>		/* fops structures */
#include <linux/slab.h>
#include "../convenient.h"

#define MODNAME   "miscdrv"

MODULE_AUTHOR("laurijssen");
MODULE_DESCRIPTION("boilerplate misc char driver");
MODULE_LICENSE("Dual MIT/GPL");
MODULE_VERSION("0.1");

#define MAX_SECRET_LEN 1024

struct driver_context {
    struct device *dev;
    int rx, tx;
    char secret[MAX_SECRET_LEN];
};

static struct driver_context *ctx;

static int open_miscdrv(struct inode *inode, struct file *filp)
{
	char *buf = kzalloc(PATH_MAX, GFP_KERNEL);

	if (unlikely(!buf))
		return -ENOMEM;
	PRINT_CTX();		// displays process (or atomic) context info

	pr_info(" opening \"%s\" now; wrt open file: f_flags = 0x%x\n",
		file_path(filp, buf, PATH_MAX), filp->f_flags);

	kfree(buf);
	return nonseekable_open(inode, filp);
}

static ssize_t read_miscdrv(struct file *filp, char __user *ubuf, size_t count, loff_t *off)
{
    struct device *dev = ctx->dev;

    if (copy_to_user(ubuf, ctx->secret, strlen(ctx->secret)+ 1))
    {
        return -EFAULT;
    }

	dev_info(dev, "numbytes read %lu\n", strlen(ubuf) + 1);

	return count;
}

static ssize_t write_miscdrv(struct file *filp, const char __user *ubuf,
			     size_t count, loff_t *off)
{
    struct device *dev = ctx->dev;
    void *kbuf = NULL;

    dev_info(dev, "writing %s\n", ubuf);

    kbuf = kvmalloc(count, GFP_KERNEL);
    memset(kbuf, 0, count);
    
    if (copy_from_user(kbuf, ubuf, count))
    {
        dev_info(dev, "secret changed %s\n", ctx->secret);
        return -EFAULT;
    }

    strscpy(ctx->secret, kbuf, count);

    dev_info(dev, "secret changed %s\n", ctx->secret);

	return count;
}

static int close_miscdrv(struct inode *inode, struct file *filp)
{
	char *buf = kzalloc(PATH_MAX, GFP_KERNEL);

	if (unlikely(!buf))
		return -ENOMEM;
	pr_info("closing \"%s\"\n", file_path(filp, buf, PATH_MAX));
	kfree(buf);

	return 0;
}

static const struct file_operations llkd_misc_fops = {
	.open = open_miscdrv,
	.read = read_miscdrv,
	.write = write_miscdrv,
	.release = close_miscdrv,
	.llseek = no_llseek,
};

static struct miscdevice llkd_miscdev = {
	.minor = MISC_DYNAMIC_MINOR,	/* kernel dynamically assigns a free minor# */
	.name = "llkd_miscdrv",
	.mode = 0666,		
	.fops = &llkd_misc_fops,
};

static int __init miscdrv_init(void)
{
	int ret;
	struct device *dev;

	ret = misc_register(&llkd_miscdev);
	if (ret != 0) {
		pr_notice("misc device registration failed, abort\n");
		return ret;
	}

	/* Retrieve the device pointer for this device */
	dev = llkd_miscdev.this_device;

    ctx = devm_kzalloc(dev, sizeof *ctx, GFP_KERNEL);
    ctx->dev = dev;
    strscpy(ctx->secret, "init", 5);

	pr_info("LLKD misc driver (major # 10) registered, minor# = %d,"
		" dev node is /dev/%s\n", llkd_miscdev.minor, llkd_miscdev.name);

	dev_info(dev, "sample dev_info(): minor# = %d\n", llkd_miscdev.minor);

	return 0;		/* success */
}

static void __exit miscdrv_exit(void)
{
	misc_deregister(&llkd_miscdev);
	pr_info("misc driver deregistered\n");
}

module_init(miscdrv_init);
module_exit(miscdrv_exit);

