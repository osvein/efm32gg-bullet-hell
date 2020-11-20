#include <linux/cdev.h>
//#include <linux/clk.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/ioport.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/types.h>

// TODO write in report about:
// * common clock framework (energy saving)
// * gpio subsystem (user space API)

/* returns err if ptr is null, 0 otherwise */
static inline int null_to_err(int err, void *ptr) {
	return ptr ? 0 : err;
}

/* returns error contained in ptr, or 0 if no error */
static inline int ptr_to_err(void *ptr) {
	return IS_ERR(ptr) ? PTR_ERR(ptr) : 0;
}

static const phys_addr_t gpad_portaddr = 0x4006048;
static const unsigned long gpad_portsize = 0x24;
static const unsigned long gpad_portoff_model = 0x04;
static const unsigned long gpad_portoff_dout = 0x0C;
static const unsigned long gpad_portoff_din = 0x1C;

static struct class *gpad_class;
static struct device *gpad_dev;
static void __iomem *gpad_port;
//static struct clk *gpad_gpioclk;
static struct cdev gpad_cdev = {
	.owner = THIS_MODULE
};

static int gpad_open(struct inode *inode, struct file *file)
{
	if ((file->f_flags & O_ACCMODE) != O_RDONLY) return -EACCES;

	/* the clock framework refcounts internally, so we call enable/disable for
	 * every open/release call, and the clock will be automatically disabled
	 * when there are no open file references
	 */
//	return clk_prepare_enable(gpad_gpioclk);
	return 0;
}

static int gpad_release(struct inode *inode, struct file *file)
{
//	clk_disable_unprepare(gpad_gpioclk);
	return 0;
}

static ssize_t gpad_read(struct file *file, char __user *buf, size_t count,
	loff_t *off
) {
	if (count < 1) return 0;
	*buf = ioread32(gpad_port + gpad_portoff_din);
	return 1;
}

/*static int gpad_mmap(struct file *file, struct vm_area_struct *vma)
{
	vma->vm_ops = &gpad_vmops;
	return 0;
}

static struct vm_operations_struct gpad_vmops = {
	.open = gpad_vmopen,
	.close = gpad_vmclose,
	.fault = gpad_vmfault,
};*/

static struct file_operations gpad_fops = {
	.owner = THIS_MODULE,
	.open = gpad_open,
	.release = gpad_release,
	.read = gpad_read,
//	.mmap = gpad_mmap,
};

/* no __exit attribute because it is called by gpad_init on failure,
 * therefore it must also not make any assumptions about which parts of init
 * have been run (e.g. check null pointers)
 */
static void gpad_exit(void)
{
	dev_t devno;
	unsigned count;

	// TODO can me assume that release has been called on all open files?
	devno = gpad_cdev.dev;
	count = gpad_cdev.count;
//	if (gpad_gpioclk) clk_put(gpad_gpioclk);
	if (gpad_class) {
		device_destroy(gpad_class, devno);
		class_destroy(gpad_class);
	}
	cdev_del(&gpad_cdev);
	if (count) unregister_chrdev_region(devno, count);
}

static int __init gpad_init(void)
{
	long ret;
	dev_t devno;

	cdev_init(&gpad_cdev, &gpad_fops);
	ret = alloc_chrdev_region(&devno, 0, 1, "gamepad");
	if (ret < 0) goto err;
	ret = cdev_add(&gpad_cdev, devno, 1);
	if (ret < 0) goto err;
	ret = ptr_to_err(gpad_class = class_create(THIS_MODULE, "gamepad"));
	if (ret < 0) goto err;
	ret = ptr_to_err(
		gpad_dev = device_create(gpad_class, NULL, devno, NULL, "gamepad")
	);
	if (ret < 0) goto err;
	ret = null_to_err(-EBUSY, devm_request_mem_region(gpad_dev,
		gpad_portaddr, gpad_portsize, "gamepad_port"
	));
	if (ret < 0) goto err;
	ret = null_to_err(-ENOMEM,
		gpad_port = devm_ioremap(gpad_dev, gpad_portaddr, gpad_portsize)
	);
	if (ret < 0) goto err;
//	ret = ptr_to_err(gpad_gpioclk = clk_get(NULL, "HFPERCLK.GPIO"));
//	if (ret < 0) goto err;
//	printk(KERN_INFO "8");

//	// TODO is it really necessary to enable the clock to write registers?
//	ret = clk_prepare_enable(gpad_gpioclk);
//	if (ret < 0) goto err;
	iowrite32(0x33333333, gpad_port + gpad_portoff_model);
	iowrite32(0xFF, gpad_port + gpad_portoff_dout);
//	clk_disable_unprepare(gpad_gpioclk);

	printk(KERN_INFO "initialized");
	return 0;
err:
	gpad_exit();
	return ret;
}

module_init(gpad_init);
module_exit(gpad_exit);

MODULE_DESCRIPTION("TDT4258 gamepad driver.");
MODULE_LICENSE("GPL");
