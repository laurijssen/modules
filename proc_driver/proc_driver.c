#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>		/* fops structures */
#include <linux/slab.h>
#include "../convenient.h"

#define MODNAME   "laurijssen"

MODULE_AUTHOR("laurijssen");
MODULE_DESCRIPTION("boilerplate misc char driver");
MODULE_LICENSE("Dual MIT/GPL");
MODULE_VERSION("0.1");

DEFINE_MUTEX(mtx);

#define MAX_SECRET_LEN 1024
#define PROC_FILE1 "proc_debug_level"
#define PROC_FILE1_PERMS 0644

static int proc_open_dbg_level(struct inode *inode, struct file *file);
static ssize_t proc_write_debug_level(
		struct file *filp, const char __user *ubuf, size_t count,
		loff_t *off);

static const struct proc_ops fops_debug_level = {
	.proc_open = proc_open_dbg_level,
	.proc_read = seq_read,
	.proc_write = proc_write_debug_level,
};

#define DEBUG_LEVEL_MIN		0
#define DEBUG_LEVEL_MAX		2
#define DEBUG_LEVEL_DEFAULT	DEBUG_LEVEL_MIN

static struct proc_dir_entry *maindir;
static struct proc_dir_entry *file1;
static int debug_level;

static ssize_t proc_write_debug_level(
		struct file *filp, const char __user *ubuf, size_t count,
		loff_t *off)
{
	char buf[12];
	int ret = count;

	if (mutex_lock_interruptible(&mtx))
		return -ERESTARTSYS;
	if (count == 0 || count > 12) {
		ret = -EINVAL;
		goto out;
	}
	if (copy_from_user(buf, ubuf, count)) {
		ret = -EFAULT;
		goto out;
	}
	buf[count - 1] = '\0';
	pr_debug("user sent: buf = %s\n", buf);
	ret = kstrtoint(buf, 0, &debug_level); /* update it! */
	if (ret)
		goto out;
	if (debug_level < DEBUG_LEVEL_MIN || debug_level > DEBUG_LEVEL_MAX) {
		pr_info("trying to set invalid value for debug_level\n"
			" [allowed range: %d-%d]\n", DEBUG_LEVEL_MIN, DEBUG_LEVEL_MAX);
		debug_level = DEBUG_LEVEL_DEFAULT;
		ret = -EFAULT;
		goto out;
	}

	ret = count;
out:
	mutex_unlock(&mtx);
	return ret;
}

static int proc_show_debug_level(struct seq_file *seq, void *v)
{
	if (mutex_lock_interruptible(&mtx))
		return -ERESTARTSYS;

	seq_printf(seq, "debug_level:%d\n", debug_level);

	mutex_unlock(&mtx);

	return 0;
}

static int proc_open_dbg_level(struct inode *inode, struct file *file)
{
	return single_open(file, proc_show_debug_level, NULL);
}

static int __init procdrv_init(void)
{
	maindir = proc_mkdir(MODNAME, NULL);
	file1 = proc_create(PROC_FILE1, PROC_FILE1_PERMS, maindir, &fops_debug_level);

	return 0;		/* success */
}

static void __exit procdrv_exit(void)
{
	remove_proc_subtree(MODNAME, NULL);
	pr_info("misc driver deregistered\n");
}

module_init(procdrv_init);
module_exit(procdrv_exit);
