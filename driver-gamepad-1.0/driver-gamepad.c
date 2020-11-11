#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>

static int gamepad_open(struct inode *inode, struct file *file)
{
	return -1;
}

static int gamepad_release(struct inode *inode, struct file *file)
{
	return -1;
}

static ssize_t gamepad_read(struct file *file, char __user *buf, size_t count,
	loff_t *off
) {
	return -1;
}

static ssize_t gamepad_write(struct file *file, char __user *buf, size_t count,
	loff_t *off
) {
	return -1;
}

static struct file_operations gamepad_fops = {
	.owner = THIS_MODULE,
	.open = gamepad_open,
	.release = gamepad_release,
	.read = gamepad_read,
	.write = gamepad_write
};

static struct cdev gamepad_cdev = {
	.owner = THIS_MODULE
};

static struct class *gamepad_class;

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

static void gamepad_exit(void)
{
	dev_t dev;
	unsigned count;

	dev = gamepad_cdev.dev;
	count = gamepad_cdev.count;
	device_destroy(&gamepad_class, dev);
	class_destroy(&gamepad_class);
	cdev_del(&gamepad_cdev);
	unregister_chrdev_region(dev, count);
}

module_init(gamepad_init);
module_exit(gamepad_cleanup);

MODULE_DESCRIPTION("TDT4258 gamepad driver.");
MODULE_LICENSE("GPL");
