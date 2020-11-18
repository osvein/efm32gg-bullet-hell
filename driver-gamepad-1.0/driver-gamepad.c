#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/ioport.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/types.h>

/* TODO remember to write in report about gpio subsystem */

static const phys_addr_t gamepad_portaddr = 0x4006048;
static const unsigned long gamepad_portsize = 0x24;
static const unsigned long gamepad_portoff_model = 0x04;
static const unsigned long gamepad_portoff_dout = 0x0C;
static const unsigned long gamepad_portoff_din = 0x1C;

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
	*buf = ioread32(gamepad_portaddr + gamepad_portoff_din);
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
static struct device *gamepad_dev;
static void __iomem *gamepad_port;
static struct clk *gamepad_gpioclk;

/**
 * Closes the gamepad and removes it from the device
*/
static void gamepad_exit(void)
{
	dev_t devno;
	unsigned count;

	devno = gamepad_cdev.dev;
	count = gamepad_cdev.count;
	if (gamepad_gpioclk) {
		clk_disable(gamepad_gpioclk);
		clk_put(gamepad_gpioclk);
	}
	if (gamepad_class) {
		device_destroy(gamepad_class, devno);
		class_destroy(gamepad_class);
	}
	cdev_del(&gamepad_cdev);
	if (count) unregister_chrdev_region(devno, count);
}

/**
 * Initializes the gamepad
 * 
 * @returns {} - 
*/
static int __init gamepad_init(void)
{
	long ret;
	dev_t devno;

	cdev_init(&gamepad_cdev, &gamepad_fops);
	ret = alloc_chrdev_region(&devno, 0, 1, "gamepad");
	if (ret < 0) goto err;
	ret = cdev_add(&gamepad_cdev, devno, 1);
	if (ret < 0) goto err;
	gamepad_class = class_create(THIS_MODULE, "gamepad"));
	if (IS_ERR(gamepad_class)) {
		ret = -PTR_ERR(gamepad_class);
		goto err;
	}
	gamepad_dev = device_create(gamepad_class, NULL, dev, NULL, "gamepad");
	if (IS_ERR(gamepad_dev)) {
		ret = -PTR_ERR(gamepad_dev);
		goto err;
	}

	if (!devm_request_mem_region(gamepad_dev, gamepad_portaddr,
		gamepad_portsize, "gamepad_port"
	)) {
		ret = -EBUSY;
		goto err;
	}
	gamepad_port = devm_ioremap(gamepad_dev, gamepad_portaddr, gamepad_portsize);
	if (!gamepad_port) {
		ret = -ENOMEM;
		goto err;
	}

	gamepad_gpioclk = clk_get(NULL, "HFPERCLK.GPIO");
	if (IS_ERR(gpioclk)) return -PTR_ERR(gpioclk);
	ret = clk_enable(gpioclk);
	if (ret < 0) goto err;
	iowrite32(0x33333333, gamepad_port + gamepad_portoff_model);
	iowrite32(0xFF, gamepad_port + gamepad_portoff_dout);
	return 0;
err:
	gamepad_exit();
	return ret;
}

module_init(gamepad_init);
module_exit(gamepad_exit);

MODULE_DESCRIPTION("TDT4258 gamepad driver.");
MODULE_LICENSE("GPL");
