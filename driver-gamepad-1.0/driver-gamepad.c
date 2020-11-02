#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>

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

static int __init gamepad_init(void)
{
	return -1;
}

static void __exit gamepad_cleanup(void)
{

}

static struct file_operations gamepad_fops = {
	.owner = THIS_MODULE,
	.open = gamepad_open,
	.release = gamepad_release,
	.read = gamepad_read,
	.write = gamepad_write
};

module_init(gamepad_init);
module_exit(gamepad_cleanup);

MODULE_DESCRIPTION("TDT4258 gamepad driver.");
MODULE_LICENSE("GPL");
