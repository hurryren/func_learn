#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h> /* printk() */
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/list.h>


#include "main.h"
#include "fops.h"

static int orange_major = 0;
static int orange_minor = 0;

static struct orange_dev *orange_dev;

static struct file_operations orange_fops = {
	.owner 	= THIS_MODULE,
	.open	= orange_open,
       	.read	= orange_read,
	.write  = orange_write,
	.release= orange_release,
};

static
void __init init_orange_dev(struct orange_dev *dev){
	dev->block_counter = 0;

	INIT_LIST_HEAD(&dev->list_entry);
	mutex_init(&dev->mutex);

	cdev_init(&dev->cdev, &orange_fops);
	dev->cdev.owner = THIS_MODULE;
}

static
int __init m_init(void){
	int err = 0;
	dev_t devno;

	printk(KERN_WARNING MODULE_NAME " is loaded\n");

	/* alloc device number */
	err = alloc_chrdev_region(&devno, orange_minor, 1, MODULE_NAME);
	if(err < 0){
		pr_debug("can not get major");
		return err;
	}

	orange_major = MAJOR(devno);

	orange_dev = kmalloc(sizeof(struct orange_dev), GFP_KERNEL);
	if(!orange_dev){
		pr_debug("Error(%d): kmalloc faild on orange\n", err);
	}

	init_orange_dev(orange_dev);

	/**
	 * The cdev_add() function will make this char device usable
	 * in userspace. If you have not ready to populate this device
	 * to its users, do not call cdev_add()
	 */
	devno = MKDEV(orange_major, orange_minor);
	err = cdev_add(&orange_dev->cdev, devno, 1);
	if(err){
		pr_debug("Error(%d): adding %s error\n", err, MODULE_NAME);
		kfree(orange_dev);
		orange_dev = NULL;
	}

	/* TODO: unregister chrdev_region here if fail */
	return 0;
}


static
void __exit m_exit(void){
	dev_t devno;
	printk(KERN_WARNING MODULE_NAME " unloaded\n");

	cdev_del(&orange_dev->cdev);
	orange_trim(orange_dev);
	kfree(orange_dev);

	devno = MKDEV(orange_major, orange_minor);
	unregister_chrdev_region(devno, 1);
}

module_init(m_init);
module_exit(m_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("orange");
MODULE_DESCRIPTION("a char device driver copy from ldd3");

