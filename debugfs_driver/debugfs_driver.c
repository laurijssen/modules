#include <linux/init.h>
#include <linux/kernel.h>
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

#define OURMODNAME "debugfs_laurijssen"
#define DBGFS_FILE1 "llkd_dbgfs_show_drvctx"
#define DBGFS_FILE2	"llkd_dbgfs_debug_level"

#define MAXUPASS 256

static ssize_t dbgfs_show_drvctx(struct file *filp, char __user *ubuf, size_t count, loff_t *fpos);

struct drv_ctx {
	int tx, rx, err, myword, power;
	u32 config1;
	u32 config2;
	u64 config3;
	char secret[128];
};

static struct drv_ctx *gdrvctx;
static int debug_level;

static struct dentry *gparent;

static const struct file_operations dbg_drvctx_fops = {
    .read = dbgfs_show_drvctx
};

static ssize_t dbgfs_show_drvctx(struct file *filp, char __user *ubuf,
				 size_t count, loff_t *fpos)
{
	struct drv_ctx *data = (struct drv_ctx *)filp->f_inode->i_private;
			// retrieve the "data" from the inode
#define MAXUPASS 256	// careful- the kernel stack is small!
	char locbuf[MAXUPASS];

	if (mutex_lock_interruptible(&mtx))
		return -ERESTARTSYS;

	data->config3 = jiffies;
	snprintf(locbuf, MAXUPASS - 1,
		 "prodname:%s\n"
		 "tx:%d,rx:%d,err:%d,myword:%d,power:%d\n"
		 "config1:0x%x,config2:0x%x,config3:0x%llx (%llu)\n"
		 "secret:%s\n",
		 OURMODNAME,
		 data->tx, data->rx, data->err, data->myword, data->power,
		 data->config1, data->config2, data->config3, data->config3,
		 data->secret);

	mutex_unlock(&mtx);
	return simple_read_from_buffer(ubuf, MAXUPASS, fpos, locbuf,
				       strlen(locbuf));
}

static struct drv_ctx *alloc_init_drvctx(void)
{
	struct drv_ctx *drvctx = NULL;

	drvctx = kzalloc(sizeof(struct drv_ctx), GFP_KERNEL);
	if (!drvctx)
		return ERR_PTR(-ENOMEM);
	drvctx->config1 = 0x0;
	drvctx->config2 = 0x48524a5f;
	drvctx->config3 = jiffies;
	drvctx->power = 1;
	strncpy(drvctx->secret, "somedata", 8);

	pr_info("allocated and init the driver context structure\n");
	return drvctx;
}

static int __init debugfsdrv_init(void)
{
    int stat = 0;

    struct dentry *file1;

    gparent = debugfs_create_dir(OURMODNAME, NULL);

	if (!gparent) {
		pr_info("debugfs_create_dir failed, aborting...\n");
		stat = PTR_ERR(gparent);
		goto out_fail_1;
	}

	gdrvctx = alloc_init_drvctx();
	if (IS_ERR(gdrvctx)) {
		pr_info("drv ctx alloc failed, aborting...\n");
		stat = PTR_ERR(gdrvctx);
		goto out_fail_2;
	}

    file1 = debugfs_create_file(DBGFS_FILE1, 0440, gparent, (void *)gdrvctx, &dbg_drvctx_fops);

	if (!file1) {
		pr_info("debugfs_create_file failed, aborting...\n");
		stat = PTR_ERR(file1);
		goto out_fail_3;
	}

	pr_debug("debugfs file 1 <debugfs_mountpt>/%s/%s created\n", OURMODNAME, DBGFS_FILE1);

    debugfs_create_u32(DBGFS_FILE2, 0644, gparent, &debug_level);

	return 0;

out_fail_3:
    kfree(gdrvctx);
out_fail_2:
    debugfs_remove_recursive(gparent);
out_fail_1:
    return stat;
}

static void __exit debugfsdrv_exit(void)
{
    kfree(gdrvctx);

    debugfs_remove_recursive(gparent);
    pr_info("removed\n");
}

module_init(debugfsdrv_init);
module_exit(debugfsdrv_exit);
