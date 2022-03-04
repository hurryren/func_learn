#ifndef _PROC_OPS_VERSION_H
#define _PROC_OPS_VERSION_H

#include<linux/version.h>

#ifdef CONFIG_COMPAT
#define __add_proc_ops_compat_ioctl(pops, fops)  \
    (pops)->proc_compat_ioctl = (fops)->compat_ioctl



#endif