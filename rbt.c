#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kprobes.h>
#include <linux/pci.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>
#include <asm/io.h>

#define enable_reboot 1

#define DEVICE_NAME "rbt"
#define CLASS_NAME "chardrv"

static dev_t first;
static struct cdev c_dev;
static struct class *cl;

static unsigned long *sys_call_table_address;

static long my_write_cr0(long value)
{
	__asm__ volatile("mov %0, %%cr0" :: "r"(value) : "memory");

	return 0;
}

#define disable_write_protection() my_write_cr0(read_cr0() & (~0x10000));

#define enable_write_protection() ({my_write_cr0(read_cr0() | (~0x10000));})

static unsigned long *sys_call_table_address;
asmlinkage int (*old_reboot_sys_call)(int, int, int, void*);

static struct kprobe kp = {
	.symbol_name = "kallsyms_lookup_name"
};

typedef unsigned long (*kallsyms_lookup_name_t)(const char *name);

unsigned long *get_system_call_table_address(void) {
	kallsyms_lookup_name_t kallsyms_lookup_name;
	register_kprobe(&kp);
	kallsyms_lookup_name = (kallsyms_lookup_name_t)kp.addr;
	unregister_kprobe(&kp);
	return (unsigned long *)kallsyms_lookup_name("sys_call_table");
}

asmlinkage int h_reboot(int magic1, int magic2, int cmd, void *args)
{
	if (enable_reboot) {
		return old_reboot_sys_call(magic1, magic2, cmd, args);
	}

	printk(KERN_NOTICE, "blocked reboot call");

	return EPERM;
}

static void hook_sys_call(void)
{
	old_reboot_sys_call = sys_call_table_address[__NR_reboot];
	disable_write_protection();
	sys_call_table_address[__NR_reboot] = (unsigned long)h_reboot;
	enable_write_protection();
	printk(KERN_NOTICE, "hooked reboot call");
}

static void restore_reboot_sys_call(void)
{
	disable_write_protection();
	sys_call_table_address[__NR_reboot] = (unsigned long)old_reboot_sys_call;
	enable_write_protection();
}

static int dev_open(struct inode *i, struct file *f)
{
	return 0;
}

static int dev_close(struct inode *i, struct file *f)
{
	return 0;
}

static ssize_t dev_read(struct file *f, char __user *buf, size_t len, loff_t *off)
{
	return 0;
}

static ssize_t dev_write(struct file *f, const char __user *buf, size_t len, loff_t *off)
{
	int i;
	char b[64] = {0};
	for (i = 0; i < len; i++)
	{
		b[i] = buf[i];
	}
	printk(b);
	return len;
}


static void __exit shutdown(void)
{
	restore_reboot_sys_call();

	cdev_del(&c_dev);
	device_destroy(cl, first);
	class_destroy(cl);
	unregister_chrdev_region(first, 1);
}

static struct file_operations hello_fops =
{
	.owner = THIS_MODULE,
	.open = dev_open,
	.release = dev_close,
	.write = dev_write,
	.read = dev_read,
};

static int __init startup(void)
{
	printk("rbt installed\n");

	if (alloc_chrdev_region(&first, 0, 1, DEVICE_NAME) < 0)
		return -1;

	if ((cl = class_create(THIS_MODULE, CLASS_NAME)) == NULL)
	{
		unregister_chrdev_region(first, 1);
		return -1;
	}

	if (device_create(cl, NULL, first, NULL, DEVICE_NAME) == NULL)
	{
		class_destroy(cl);
		unregister_chrdev_region(first, 1);
		return -1;
	}

	cdev_init(&c_dev, &hello_fops);

	if (cdev_add(&c_dev, first, 1) == -1)
	{
		printk("fail rbt driver" );
		device_destroy(cl, first);
		class_destroy(cl);
		unregister_chrdev_region(first, 1);
		return -1;
	}

	sys_call_table_address = get_system_call_table_address();

	hook_sys_call();

	return 0;
}


module_init(startup);
module_exit(shutdown);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("SL");
MODULE_DESCRIPTION("rbt");
MODULE_VERSION("1.0.0");

