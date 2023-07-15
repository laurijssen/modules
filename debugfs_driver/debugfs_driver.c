#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/seq_file.h>
#include <linux/miscdevice.h>
#include <linux/debugfs.h>
#include <linux/fs.h>		/* fops structures */
#include <linux/slab.h>
#include "../convenient.h"

MODULE_AUTHOR("laurijssen");
MODULE_DESCRIPTION("debugfs driver");
MODULE_LICENSE("Dual MIT/GPL");
MODULE_VERSION("0.1");

DEFINE_MUTEX(mtx);

#define OUR_MODNAME "debugfs_laurijssen"
#define DBGFS_FILE1 "llkd_dbgfs_show_drvctx"

static struct dentry *gparent;

static const struct file_operations dbg_drvctx_fops = {
    .read = dbgfs_show_drvctx
};

static ssize_t dbgfs_show_drvctx(struct file *filp, char __user *ubuf, size_t count, loff_t *fpos)
{
    struct drv_ctx *data = (struct drv_ctx *)filp->f_inode->i_private;
#define MAXUPASS 256
    char locBuff[MAXUPASS];

    if (mutex_lock_interruptible(&mtx))
        return -ERESTARTSYS;

    data->config3 = jiffies;
    snprintf(locBuff, MAXUPASS - 1, "");
}

static int __init debugfsdrv_init(void)
{
    int stat = 0;

    struct dentry *file1, *file2;

    gparent = debugfs_create_dir(OUR_MODNAME, NULL);

    debugfs_create_file(DBGFS_FILE1, 0440, gparent, (void *)gdrvctx, &dbg_drvctx_fops);

	return 0;		/* success */
}

static void __exit debugfsdrv_exit(void)
{
}

module_init(debugfsdrv_init);
module_exit(debugfsdrv_exit);
