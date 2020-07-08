#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/pci.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>
#include <asm/io.h>

#define DEVICE_NAME "HelloWorld"
#define CLASS_NAME "chardrv"

static dev_t first;
static struct cdev c_dev;
static struct class *cl;

static int hello_open(struct inode *i, struct file *f)
{
	printk("hello world open()");
	return 0;
}

static int hello_close(struct inode *i, struct file *f)
{
	printk("hello world close()");
	return 0;
}

static ssize_t hello_read(struct file *f, char __user *buf, size_t len, loff_t *off)
{
	printk("hello world read()");
	return 0;
}

static ssize_t hello_write(struct file *f, const char __user *buf, size_t len, loff_t *off)
{
	printk("hello world write()");
	return len;
}


static void __exit hello_exit(void)
{
	printk("Goodbye world\n");

	cdev_del(&c_dev);
	device_destroy(cl, first);
	class_destroy(cl);
	unregister_chrdev_region(first, 1);
}

static struct file_operations hello_fops =
{
	.owner = THIS_MODULE,
	.open = hello_open,
	.release = hello_close,
	.write = hello_write,
	.read = hello_read,
};

static int __init hello_init(void)
{
	printk("Hello, world\n");

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

	printk("3");
	cdev_init(&c_dev, &hello_fops);

	if (cdev_add(&c_dev, first, 1) == -1)
	{
		printk("fail" );
		device_destroy(cl, first);
		class_destroy(cl);
		unregister_chrdev_region(first, 1);
		return -1;
	}
	printk("5");
	printk("4");
	return 0;
}


module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("SL");
MODULE_DESCRIPTION("Hello");
MODULE_VERSION("1.0.0");

