#include <linux/init.h>
#include <linux/kernel.h> /* printk() */
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>

#include <asm/uaccess.h> /* copy_*_user */



struct orange_qset {
    void **data;
    struct orange_qset *next;
};

struct orange_dev {
    struct orange_qset *data;   /* pointer to first quantum set */
    int quantum;                /* the current quantum size */
    int qset;                   /* the current array size */
    unsigned long size;         /* amount of data stored here */
    unsigned int access_key;    /* used by uid and priv */
    struct semaphore sem;       /* mutual exclusion semaphore */
    struct cdev cdev;           /* char device structure */
};

/*
 * parameters which can be set at load time.
 */
int orange_major = 0;
int orange_minor = 0;
int orange_nr_devs = 4;
int orange_quantum = 4000;
int orange_qset = 1000;


MODULE_AUTHOR("orange");
MODULE_LICENSE("Dual BSD/GPL");


/*
 * empty out the orange device;must be called with the device
 * semaphore held.
 */
int orange_trim(struct orange_dev *dev){
	struct orange_qset *next, *dptr;
	int qset = dev->qset;
	int i;
	for(dptr = dev->data;dptr;dptr = next){
		if(dptr->data){
			for(i = 0; i < qset; i++)
				kfree(dptr->data[i]);
			kfree(dptr->data);
			dptr->data = NULL;
		}
		next = dptr->next;
		kfree(dptr);
	}
	dev->size = 0;
	dev->quantum = orange_quantum;
	dev->qset = orange_qset;
	dev->data = NULL;
	return 0;
}



/*
 * set up the char_dev structure for this device
 */
static void orange_setup_cdev(struct orange_dev *dev, int index)
{
	int err;
	int devno = MKDEV(orange_major, orange_minor + index);

	cdev_init(&dev->cdev, &orange_fops);
	dev->cdev->owner = THIS_MODULE;
	dev->cdev.ops = &orange_fops;
	err = cdev_add(&dev->cdev, devno, 1);
	if(err)
		printk(KERNEL_NOTIC "Error %d adding orange %d",err, index);
}



static int orange_init(void){
	int result;
	int i;
	dev_t dev=0;

	/* asking for a dynamic major */
	result = alloc_chrdev_region(&dev, orange_minor, orange_nr_devs, "orange");
	orange_major = MAJOR(dev);
}

static void orange_exit(void){
	printk(KERN_ALERT "Goodbye, cruel world\n");
}

module_init(orange_init);
module_exit(orange_exit);


