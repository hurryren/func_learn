https://lwn.net/Kernel/LDD3/











### The design of scull

The first step of driver writing is defining the capabilities (the mechanism) the driver will offer to user programs. Since our device is part of the computer’s memory, we are free to do what we want with it. It can be a sequential or random-access device, one device or many, and so on.

The scull source implements the following devices. Each kind of device implemented by the module is referred to as a type.

>   scull0 to scull3
>
>   >   four devices, each consisting of a memory area that is both global and persistent. Global means that if the device is opended multiple times, the data contained within the device is shared by all the file descriptors that opened it. Persistent means that if the device is closed and reopened, data is not lost. This device can be fun to work with, because it can be accessed and tested using conventional commands, such as cp, cat, and shell IO redirection.
>
>   scullpipe0 top scullpipe3
>
>   >   Four FIFO devices, which act like pipes. One process reads what another process writes. If multiple processes read the same device, they contend for data. The internals of scullpipe will show how blocking and non-blocking read and write can be implemented without having to resort to interupts. Although real drivers synchronize with their devices using hardware interrupts, the topic of blocking and nonblocking operations is an important one and is eparate from interrupts handling.
>
>   scullsingle
>
>   scullpriv
>
>   sculluid
>
>   scullwuid
>
>   >   these devices are similar to scull0 but with some limitations on when an open is permitted. The first (scullsingle) allows only one process at a time to use the driver, whereas scullpriv is private to each virtual console(or X terminal session), because processes on each console/terminal get different memory areas. sculluid and scullwuid can be opened multiple times, but only by one user at a time; the former returns an error of device busy if another user is locking the device, whereas the latte implements blocking open. These variations of scull would appear to be confusing policy an dmechanism, but they are worth looking at, because some real-life devices require this sort of management.



### Major and minor numbers

char devices are accesses through names in the filesystem. Those names are called special files or device files or simple nodes of the filesystem tree; they are conventionally located in the /dev directory. Special files for char drivers are identified by a ‘c’ in the first column of the output of ls -l. block devices appear in /dev as well, but they are identified by a ‘b’. The focus of this chapter is on char devices, but much of the following infomation applies to block devices as well.

Traditionally, the major number identifies the driver associated with the device. For example, /dev/null and /dev/zero are both managed by driver 1, whereas virual consoles and serial termianls are managed by driver 4; similarly,both vcs1 and vcsa1 devices are managed by driver 7. modern linux kernels allow multiple drivers to share major numbers, but most devices that you will see are still organized on the one-major-onedriver principle.

The minor number is used by the kernel to determine exactly which device is being referred to. Depending on how your driver is written (as we will see below), you can either get a direct pointer to your device from  the kernel, or you can use the minor number yourself as an index into a local array of devices. Either way, the kernel itself knows almost nothing about minor numbers beyong the fact that they refer to devices implemented by your driver.

### The internal representaion of device numbers

within the kernel, the dev_t type (define in <linux/types.h>) is used to hold device numbers--both the major and minor parts. As of version 2.6.0 of the kernel, dev_t is a 32-bit quantity with 12 bits set aside for the major number and 20 for the minor number. You code should, of course, never make any asumptions about the internal orgnazation of device numbers; it should, instead, make use of a set of macros found in <linux/kdev_t.h>.



The poll method is the back end of three system calls: poll, epoll, and select, all of which are used to query whether a red or write to one or more file descriptors would block. The poll method should return a bit mask indicating whether non-blocking reds or writes are possible, and, possible, provide the kernel with information that can be used to put the calling process to sleep until io becomes possible. If a driver leaves its poll method null, the device is assumed to be both readable and writable without blocking.

### the file structure

struct file, defined in <linuxc/fs.h>, is the second most important data structure used in device drivers. Note that a file has nothing to do with the FILE pointers of user-space programs. A FILE is deifned in the c library and never appears in kernel code.  A stuct file, on the other hand, is a kernel structure that never appeats in user programs.

The file structure represents an open file. (it is not specific to device drivers; every open file in the system has an associated struct file in kernel space.) It is created by the kernal on open and is passed to any function that operates on the file, until the last close. After all instances of the file are closed, the kernel releases the data structure.

In the kernel sources, a pointer to struct file is usually called either file or filp(file pointer). We will consistently call the pointer filp to prevent ambiguities with the structure itself.

The most important fields of struct file are shown below:

mode_t fmod:

>   the file mode identifies the fil as either readable or writable (or both), by means of the bits FMODE_READ and FMODE_WRITE. You might want to check this field for read/write permission in your open or icctl function, but you do not need to check permissions for read and write, because the kernel checks before invoking you method. An attempt to read or write when the file has not been opened for that type of access is rejected without the driver even knowing about it.

loff_t f_pos:

>   The current reading or writing position. loff_t  is a 64-bit value on all platforms (long long in gcc terminology). The driver can read this value if it needs to know the current position in the file but should normally change it; read and write should update a position using the pointer they receive as the last argument instead  of acting on filp->f_pos directly. The one exception to this rule is in the llseek method, the purpose of which to change the file position.

unsigned int f_flag:

>   These are the file flags, such as O_RDONLY, O_NONBLOCK, and O_SYNC. A driver should check the O_NONBLOCK flag to see if nonblocking operation has been requested; the other flags are seldom used.  In particular, read/write permission should be checked using f_mode rather than f_flags. All the flags are defined in the header <linux/fcntl.h>

struct file_operations *f_op:

>   the operations associated with the file. The kernel assigns the pointer as part of its implementation of open and then reads is when it needs to dispatch any operations. The value in filp->f_op is never saved by the kernel for later reference; this means that you can change the file operations associated with your file, and the new methgods will be effective after you return to the caller. For example, the code for open associated with major number 1, substitutes she operations in filp->f_op depending on the minor number being opened. The practice allows the implementation of serveral behavious under the same major number without introducing overhead at each system call. The ability to replace the file opearations is the kernel equivalent of method overriding in object-oriented programming.

void *private_data:

>   the open system call sets this pointer to NULL before calling the open method for the driver. You are free to make its own use of the field or to ignore it; you can use the field to point allocated data, but then you must remember to free that memory in the release method before the file structure is destroyed by the kernel. private_data is a useful resource for preserving state information across ystem calls and is used by most of our sample modules.

struct dentry *f_dentry:

>   The directory entry structure associates with the file. Device driver writers normally need not concern themselves with dentry structures, other tham to access the inode structure as filp->f_dentry->d_inode.



### the inode structure

The inode structure is used by the kernel internally to represent files. Therefore, it is different from the file structure that represents an open file descriptor. There can be numerous file stuctures representing multiple open descriptors on a single file, but they all point to a single inode structure.

dev_t i_rdev:

>   For inodes that represent device files, this field contains the actual device number. 

struct cdev *icdev:*

>   struct cdev is the kernel’s internal structure that represents char devices; this field contains a pointer to that structure when the indoe refers to a char device file.



### the open method

The open method is provide for a driver to do any initialization in preparation for later operations. In most deivers, open should perform the following tasks:

-   check for device-specific errors (such as device-not-ready or similar hardware problems)
-   Initialize the device if it is being opended for the first time.
-   Update the f_op pointer, if necessary
-   allocate and fill any data structure to be put in filp->private_data

The first order of business,however, is usually to identify which device is being opened. Remember that the prototype for the open method is:

```c
int (*open)(struct inode *inode, struct file *filp);
```

the inode argument has the information we need in the form of its i_cdev field, which contains the cdev structure we set up before. The only problem is that we do not normally want the cdev structure itself, we want the orange_dev structure that contains that cdev structure. The C language lets programmers play all sorts of tricks to make that kind of conversion; programming such tricks is error prone.

```
container_of(pointer, container_type, container_field)
```

This macro takes a pointer to a field of type container_field, within a structure of type container_type, and returns a pointer to the containing structure. In orange_open. this macro is used to find the appropriate device structure:

```c
struct orange_dev *dev;  /* device information */
dev = container_of(inode->icdev, struct orange_dev, cdev);
filp->private_data = dev;  /* for other methods */
```

 Once is has found the orange_dev structure, orange stores a pointer to it in the private_data field of the file structure for easier access in the future.



### the release method

The role of the release mathod is the reverse of open.

-   Deallocate anything that open allocated in filp->private_data

-   shut down the device on last close





### quick reference

```c
#include <linux/types.h>
dev_t   /* dev_t is the type used to represent device numbers within the kernel. */
int MAJOR(dev_t dev);
int MINOR(dev_t dev); /* macros that extract the major and minor numbers form a device number. */
dev_t MKDEV(unsigned int major, unsigned int minor); /* macro that builds a dev_t data item from the major and minor numbers. */

#include <linux/fs.h> /* the filesystem header is the header required for writing device drivers. Many important functions and data structures are declared in here. */
int register_chrdev_region(dev_t first, unsigned int count, char *name);
int alloc_chrdev_region(dev_t *dev, unsigned int firstminor, unsigned int count, char *name);
void unregister_chrdev_region(dev_t first, unsigned int count); /* functions that allow a driver to allocate and free ranges of device numbers. register_chedev_region should be used when the desired major number is known in advance; for dynamic allocation, use alloc_chrdev_region instead. */
int register_chrdev(unsigned int major, const char *name, struct file_operations *fops); /* the old(pre-2.6) char devices registration routine. It is emulates in the 2.6 kernel but should not be used for new code. If the major number is not 0, it is used unchanged otherwise a dynamic number is assigned for this device. */
int unregister_chrdev(unsigned int major, const char *name); /* funciton that undoes a registration made with register_chrdev. Both major and the name string must contain the same values that were used to register the driver. */
struct file_operations;
struct file;
struct inode; /* three important data structure used by most device drivers. The file_operations structure holds a char driver's methods; struct file represents an open file, and struct inode represents a file on disk. */
#include <linux/cdev.h>
struct cdev *cdev_alloc(void);
void cdev_init5(struct cdev *dev, struct file_operations *fops);
int cdev_add(struct cdev *dev, dev_t num, unsigned int count);
void cdev_del(struct cdev *dev); /* functions for the management of cdev structures, which represent char devices within the kernel. */
#include <linux/kernel.h>
container_of(pointer, type, field); /* a convenience macro that may be used to obtain a pointer to a structure from a pointer ro some other structure contained within it. */
#include<asm/uaccess.h> /* this include file declares functions used by kernle code to move data to amd from user space. */
unsigned long copy_from_user(void *to, const void *from, unsigned long count);
unsigned long copy_to_user(void *to, const void *from, unsigned long count); /* copy data between user space and kernel space. */


```

