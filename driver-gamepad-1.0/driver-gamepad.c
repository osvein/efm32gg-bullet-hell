#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>

#include "efm32gg.h"

/* TODO remember to write in report about gpio subsystem */

char gpio;

/**
 * Opens the gamepad
 * 
 * @param {struct inode} *inode - 
 * @param {struct file} *file - 
 * 
 * @returns {???} - 
*/
static int gamepad_open(struct inode *inode, struct file *file)
{
	if ((file->f_flags & O_ACCMODE) != O_RDONLY) return -EACCES;
	*CMU_HFPERCLKEN0 |= CMU2_HFPERCLKEN0_GPIO; /* enable GPIO clock */
	*GPIO_PC_MODEL = 0x33333333; /* input with filter and pull */
	*GPIO_PC_DOUT = 0xFF; /* pull up */
	return 0;
}

/**
 * Read the input from the gamepad
 * 
 * @param {struct file} *file - the location of the file to write the input to
 * @param {char ???}
 * @param {size_t} count - 
 * @param {loff_t} *off - 
 * 
 * @return {int} - 
*/
static ssize_t gamepad_read(struct file *file, char __user *buf, size_t count,
	loff_t *off
) {
	if (count < 1) return 0;
	*buf = *GPIO_PC_DIN;
	return 1;
}

/*static int gamepad_mmap(struct file *file, struct vm_area_struct *vma)
{
	vma->vm_ops = &gamepad_vmops;
	return 0;
}

static struct vm_operations_struct gamepad_vmops = {
	.open = gamepad_vmopen,
	.close = gamepad_vmclose,
	.fault = gamepad_vmfault,
};*/

/**
 * Registers the functions so that the kernel knows how they are invoked
*/
static struct file_operations gamepad_fops = {
	.owner = THIS_MODULE,
	.open = gamepad_open,
	.read = gamepad_read,
//	.mmap = gamepad_mmap,
};

static struct cdev gamepad_cdev = {
	.owner = THIS_MODULE
};

static struct class *gamepad_class;

/**
 * Closes the gamepad and removes it from the device
*/
static void gamepad_exit(void)
{
	dev_t dev;
	unsigned count;

	dev = gamepad_cdev.dev;
	count = gamepad_cdev.count;
	device_destroy(gamepad_class, dev);
	class_destroy(gamepad_class);
	cdev_del(&gamepad_cdev);
	unregister_chrdev_region(dev, count);
}

/**
 * Initializes the gamepad
 * 
 * @returns {} - 
*/
static int __init gamepad_init(void)
{
	int ret;
	dev_t dev;

	cdev_init(&gamepad_cdev, &gamepad_fops);
	ret = alloc_chrdev_region(&dev, 0, 1, "gamepad");
	if (ret < 0) goto err;
	ret = cdev_add(&gamepad_cdev, dev, 1);
	if (ret < 0) goto err;
	gamepad_class = class_create(THIS_MODULE, "gamepad");
	if (IS_ERR(gamepad_class)) goto err;
	device_create(gamepad_class, NULL, dev, NULL, "gamepad");
	return 0;
err:
	gamepad_exit();
	return ret;
}

module_init(gamepad_init);
module_exit(gamepad_exit);

MODULE_DESCRIPTION("TDT4258 gamepad driver.");
MODULE_LICENSE("GPL");
