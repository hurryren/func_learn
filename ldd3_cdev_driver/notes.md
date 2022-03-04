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