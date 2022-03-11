# ioctl <ldd3>

用户空间的 ioctl() 系统调用原型：

```c
int ioctl(int fd, unsigned long cmd, ...);
```

... 表示一个可选参数，用点是为了在编译时防止编译器进行类型检查。



驱动程序中的 ioctl() 与用户空间不同：

```c
int (*ioctl)(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg);
```

inode 和 filp 两个指针的值对应于应用程序传递的文件描述符 fd, 这和传给 open 方法的参数一样。 参数 cmd 由用户空间不经修改地传递给驱动程序，可选的 arg 参数则无论用户程序使用的是指针还是整数值，它都以 unsigned long 的形式传递给驱动程序。如果调用程序没有传递第三个参数，那么驱动程序所接收的 arg 参数就出在未定义状态。对于这附加参数就处在未定义状态。由于对这个附加参数的类型检查被关闭了，所以如果为 ioctl 传递一个非法参数，编译器是无法报警的，相关联的程序错误很难被发现。

为了防止对错误的设备使用正确的命令，ioctl 的命令号应该在系统范围内唯一。

定义号码的新方法使用了 4 个字段，

1.   type: 8bit (_IOC_TYPEBITS)
2.   number: 8bits (_IOC_NRBITS)
3.   direction: 如果相关命令涉及到数据的传输，则该位字段定义数据传输的方向。可以使用的值包括 _IOC_NONE(没有数据传输)，\_IOC_READ、\_IOC_WRITE 以及 \_IOC_READ | _IOC_WRITE(双向数据传输)。数据传输是从应用程序的角度看的，也就是说，IOC_READ意味着从设备中读取数据，所以驱动程序必须向用户空间写入数据。
4.   size: 所设计的用户数据大小。这个字段的宽度和体系结构有关，通常是13位或者14位，具体可通过宏 _IOC_SIZEBITS找到针对特定体系结构的具体数值。内核不会检查这个位字段。对该字段的正确使用可以帮助我们检查用户空间程序的错误。

<linux/ioctl.h> 中包含的 <asm/ioctl.h> 头文件定义了一些构造命令编号的宏： 

-   _IO(type, nr) 用于构造无参数的命令编号；_
-   \_IOR(type, nr, datatype) 用于构造从驱动程序中读取的数据的命令编号； 
-   _IOW(type, nr, datatype) 用于写入数据的命令;
-   _IOWR(type,nr, datatype) 用于双向传输。 type 和 number 位字段通过参数传入，而 size 位字段通过对 datatype 参数取得 sizeof 获得。 



## 预定义命令

尽管 ioctl 系统调用绝大部分用户操作设备，但还是有一些命令是可以由内核识别的。要注意，当这些命令用于我们的设备时，她们会在我们自己的文件操作被调用之前被解码。所以如果选用了与这些预定义命令相同的编号，就永远不会收到该命令的请求，而且由于 ioctl 编号冲突，应用程序的行为将无法预测。

预定义命令分为三组：

-   可用于任务文件 （普通、设备、FIFO 和套接字）的命令
-   只用于普通文件的命令
-   特定于文件系统的命令

最后一组命令只能在宿主文件系统上执行。

下列 ioctl 命令对任务文件 （包括设备特定文件）都是预定义的：

-   FIOCLEX：设置执行时关闭标志（File Ioctl Close on Exec）。设置了这个标志之后，当调用进程执行一个新程序时，文件描述符将被关闭。
-   FIONCLEX：清除执行时关闭标志（File Ioctl Not close on Exec）。该命令将恢复通常的文件行为，并撤销上述 FIOCLEX 命令所做的工作。
-   FIOASYNC：设置或复位文件异步通知。这两个动作都可以通过 fcntl 完成，实际上没有人会使用这个命令。
-   FIOQSIZE：该命令返回文件或目录的大小。不过，当用于设备文件时，会导致 ENOTTY 错误返回。
-   FIONBIO：意指 “File Ioctl Non-blocking IO ”，即“文件 ioctl 非阻塞型 IO”。该调用修改 filp->f_flags 中的 O_NONBLOCK 标志。传递给系统调用的第三个参数指明了是设置还是清楚该标志。注意：修改这个标志的常用方法是由 fcntl 系统调用使用 F_SETFL 命令来完成。

上文提到的 fcntl 系统调用，和 ioctl 在某些方面比较像。 fnctl 调用也要传递一个命令参数和一个附加的可选参数。



## ioctl 参数

copy_from_user 和 copy_to_user 两个函数可以安全的与用户空间交换数据。这两个函数也可以在 ioctl 中使用，但是因为 ioctl 调用通常涉及到小的数据项，因此可以通过其他方法更方便的操作。为此，我们首先要通过函数 access_ok 验证地址 （而不传输数据），该函数在 <asm/uaccess.h> 中声明：

```c
int access_ok(int type, const void *addr, unsigned long size);
```

第一个参数应该是 VERIFU_READ 或 VERIFY_WRITE，取决于要执行的动作是读取还是写入用户空间内存区。addr 参数是一个用户空间地址，size 是字节数。例如，如果 ioctl 要从用户空间读取一个整数， size 就是 sizeof(int)。如果在执行地址处纪要读取又要写入，则应该使用 VERIFY_WRITE，因为他是 VERIFY_READ 的超集。

access_ok 并没有完成验证内存的全部工作，而只检查了所引用的内存是否位于进程有对应访问权限的区域内，特别是要确保访问地址没有指向内核空间的内存区。另外大多数驱动程序代码中都不需要真正调用 access_ok，因为内存管理程序会处理它。





# ioctl <kernel.org>

[address](https://docs.kernel.org/driver-api/ioctl.html)

https://docs.kernel.org/driver-api/ioctl.html



## ioctl based interfaces

ioctl() is the most common way for applications to interface with device drivers. It is flexible and easily extended by adding neew commands and can be passed through character devices, block devices as well as sockets and other special file desctriptors.

However, it is also very easy to get ioctl command definitions wrong, and hard to fix them later without breaking existing applications, so this documentation tries to help developers get it it right.

## command number definitions

The command number, or request number, is the second argument pased to the ioctl system call. While this can be any 32-bit number that uniquely that uniquely identifies an action for particular driver, there are a number of conventions around defining them.

*include/uapi/asm-generic/ioctl.h* provides four macros for defining ioctl commands that follow modern conventions: *_IO, _IOR, _IOW*, and *_IOWR*. These should be used for all new commands, with the correct parameters:

*_IO/ _IOR / _IOW / _IOWR*:

>   the macro name specifies how the argument will be used. It may be a pointer to data to be passed into the kernel (_IOW), out of the kernel (_IOR), or both (_IOWR). _IO can indicate either commands with no argument or those passing an integer. it is recommendes to only use _IO for commands without aruments, and use pointers for passing data.

*type*:

>   An 8-bit number, often a character literal, specific to a subsystem ot drive, and listed in [Ioctl numbers](https://docs.kernel.org/userspace-api/ioctl/ioctl-number.html)

*nr*:

>   an 8-bit number identifying the specific command, unique for a give value of type

*data_type*:

>   the name of the data type pointed to by the argument, the command number encodes the **sizeof(data_type)** value in a 13-bit or 14-bit interger, leading to a limit of 8191 bytes for the maximum sizeof the argument. Note: do not pass sizeof(data_type) type into _IOR/ _IOW /IOWR, as that will lead to encoding sizeof(sizeof(data_type)).



## interface versions

some subsystems use version numbers in data structures to overload commands with different interpretations of the argument.

This is generally a bad idea, since changes to existing commands tend to break existing applications.

A better approach is to add a new ioctl command with a new number. The old command still needs to be implemented in the kernel for compatibility, but this can be a wrapper around the new implemntation.

## return code

ioctl commands can return negative error codes as documented in errno(3); these get turned into errno values in user space. On success, the return code should be zero. It is also possible but not recommended to return a positive long value.

When the ioctl callback is called with an unknown command number, the handler returns either -ENOTTY or -ENOIOCTLCMD, which also results in -ENOTTY being returned from the system call. Some subsystems return -Enosys or -EINVAL here for historic reasons, but this is wrong.

## timestamps

Traditionally, timestamps and timeout values are passes as struct timespec or struct timeval, but these are problematic because of incompatibel definitions of these structures in user space after the move to 64-bit time_t.

The struct __kernel_timespec type can be used instead to be embedded in other data structures when separate second/nanosecond values are desired, or passed to user space directly, This is still not ideal thoudh, as the structure matches neither the kenel’s timespec64 nor the user space timespec exactly. The get_timespec64() and put_timespec64() helper funcitons can be used to ensure that the layout remains compatible with user space and the paddign is treated correctly.

As it is cheap to convert seconds to nanoseconds, but the opposite requires an expensive 64-bit division, a simple __u64 nanosecond value can be simpler and more eddicient.

Timeout values and timestamps should ideally use CLOCK_MONOTONIC time, as returned by ktime_get_ns() or ktime_get_ts64(), unlike CLOCK_REALTIMNE, this makes the timestamps immune from from jumping backwarrds ot forwads due to leap second adjustments and clock_settime calls.

ktime_get_real_ns() can be used for CLOCK_REALTIE timestamps that need to be persistent across a reboot or between multiple machines.





