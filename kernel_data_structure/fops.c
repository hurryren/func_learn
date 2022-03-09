#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/errno.h>
#include <asm/atomic.h>
#include <asm/uaccess.h>

#include "main.h"
#include "fops.h"

int orange_open(struct inode *inode, struct file *filp)
{
	struct orange_dev *dev;

	pr_debug("%s() is invoked\n", __FUNCTION__);

	dev = container_of(inode->i_cdev, struct orange_dev, cdev);
	filp->private_data = dev;

	if ((filp->f_flags & O_ACCMODE) ==O_WRONLY) {
		if(mutex_lock_interruptible(&dev->mutex))
			return -ERESTARTSYS;
		orange_trim(dev);
		mutex_unlock(&dev->mutex);
	}

	return 0;
}

int orange_release(struct inode *inode, struct file *filp)
{
	pr_debug("%s() is invoked\n", __FUNCTION__);
	return 0;
}

ssize_t orange_read(struct  file *filp, char __user *buff, size_t count, loff_t *f_pos)
{
	struct orange_dev *dev = filp->private_data;
	struct data_block *dblock = NULL;
	loff_t retval = -ENOMEM;
	struct list_head *plist = NULL;
	int offset = *f_pos;

	pr_debug("%s() is invoked\n", __FUNCTION__);
	
	if(count != 1){
		pr_debug("read count must be 1, but got %lld\n",count);
		/**
		 * the file read function of python cannot request 1 byte,
		 * so if not, set it by hand.
		 */
		count = 1;
		/* return -EFAULT; */
	}

	if(mutex_lock_interruptible(&dev->mutex))
		return -ERESTARTSYS;

	if(offset + 1 > dev->block_counter) {
		retval = 0;
		goto end_of_file;
	}

	plist = &dev->list_entry;
	for(int i = 0; i< offset + 1; ++i){
		plist = plist->next;
	}

	dblock = list_entry(plist, struct data_block, data_list);


	if(copy_to_user(buff, &(dblock->data), 1)) {
		retval = -EFAULT;
		goto cpy_user_error;
	}

	retval = count;
	*f_pos += count;

end_of_file:
cpy_user_error:
	pr_debug("Rd pos = %lld, block_counter = %d\n", *f_pos, dev->block_counter);
	mutex_unlock(&dev->mutex);
	return retval;
}


ssize_t orange_write(struct file *filp, const char __user *buff, size_t count, loff_t *f_pos){
	struct orange_dev *dev = filp->private_data;
	struct data_block *dblock = NULL;
	loff_t retval = -ENOMEM;
	

	pr_debug("%s() is invoked\n", __FUNCTION__);

	/**
	 * 每次只写一字节 
	 */
	if (count != 1) {
		pr_debug("write count must be 1, but got %ld\n",count);
		return -EFAULT;
	}

	/**
	 * print buff address
	 */
	pr_debug("write user buff address is [%p]\n", buff);

	
	if(mutex_lock_interruptible(&dev->mutex))
		return -ERESTARTSYS;

	/*
	 * For simplicity, we write one block each write request.
	 */
	if(!(dblock = kmalloc(sizeof(struct data_block), GFP_KERNEL)))
		goto malloc_error;
	memset(dblock, 0, sizeof(struct data_block));
	INIT_LIST_HEAD(&dblock->data_list);
	list_add_tail(&dblock->data_list, &dev->list_entry);
	dev->block_counter++;
	
	pr_debug("after write pos finish,block_counter=[%d]\n", dev->block_counter);

	dblock = list_last_entry(&dev->list_entry, struct data_block, data_list);
	

	
	if(copy_from_user(&(dblock->data), buff, 1)) {
		retval = -EFAULT;
		pr_debug("copy from user error\n");
		goto cpy_user_error;
	}
	
	pr_debug("after write pos finish, the write data = [%c]\n", dblock->data);

	retval = count;
	*f_pos += count;


malloc_error:
cpy_user_error:
	pr_debug("WR pos = %lld, block_counter = %d, write %lu bytes\n", *f_pos, dev->block_counter, count);

	mutex_unlock(&dev->mutex);
	return retval;
}


void orange_trim(struct orange_dev *dev){
	struct data_block *cur = NULL, *tmp = NULL;

	pr_debug("%s() is invoked\n", __FUNCTION__);
	
	orange_print_list(dev);

	list_for_each_entry_safe(cur, tmp, &dev->list_entry, data_list) {
		list_del(&cur->data_list);
		memset(cur, 0, sizeof(*cur));
		kfree(cur);
	}
	dev->block_counter = 0;
}

void orange_print_list(struct orange_dev *dev){
	struct data_block *cur= NULL;
	struct data_block *tmp = NULL;
	int cnt = 0;
	pr_debug("%s() is invoked\n", __FUNCTION__);

	pr_debug("%d elements in orange_dev\n",dev->block_counter);
	list_for_each_entry_safe(cur, tmp, &dev->list_entry, data_list) {
		pr_debug("data[%d] = %c \n",cnt++,cur->data);
	}
}
