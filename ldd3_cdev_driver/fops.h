#ifndef _FOPS_H_
#define _FOPS_H_

extern int orange_open(struct inode *inode, struct file *filp);
extern int orange_release(struct inode *inode, struct file *filp);
extern ssize_t orange_read(struct file *flip, char __user *buff, size_t count, loff_t *offp);
extern ssize_t orange_write(struct file *filp, const char __user *buff, size_t count, loff_t *offp);extern void orange_trim(struct orange_dev *dev);


#endif
