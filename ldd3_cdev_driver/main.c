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

static struct orange_dev *orange_dev[ORANGE_NR_DEVS];

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

	INIT_LIST_HEAD(&dev->block_list);
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
	err = alloc_chrdev_region(&devno, orange_minor, ORANGE_NR_DEVS, MODULE_NAME);
	if(err < 0){
		pr_debug("can not get major");
		return err;
	}

	orange_major = MAJOR(devno);

	for (int i = 0; i < ORANGE_NR_DEVS; i++) {
		orange_dev[i] = kmalloc(sizeof(struct orange_dev), GFP_KERNEL);
		if(!orange_dev[i]){
			pr_debug("Error(%d): kmalloc faild on orange%d\n", err, i);
			continue;
		}

		init_orange_dev(orange_dev[i]);

		/*
		 * The cdev_add() function will make this char device usable
		 * in userspace. If you have not ready to populate this device
		 * to its users, do not call cdev_add()
		 */
		devno = MKDEV(orange_major, orange_minor + i);
		err = cdev_add(&orange_dev[i]->cdev, devno, 1);
		if(err){
			pr_debug("Error(%d): adding %s%d error\n", err, MODULE_NAME, i);
			kfree(orange_dev[i]);
			orange_dev[i] = NULL;
		}
		
	}

	/* TODO: unregister chrdev_region here if fail */
	return 0;
}


static 
void __exit m_exit(void){
	dev_t devno;
	printk(KERN_WARNING MODULE_NAME " unloaded\n");

	for (int i = 0; i < ORANGE_NR_DEVS; i++){
		cdev_del(&orange_dev[i]->cdev);
		orange_trim(orange_dev[i]);
		kfree(orange_dev[i]);
	}

	devno = MKDEV(orange_major, orange_minor);
	unregister_chrdev_region(devno, ORANGE_NR_DEVS);
}

module_init(m_init);
module_exit(m_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("orange");
MODULE_DESCRIPTION("a copy char device driver copy from ldd3");

