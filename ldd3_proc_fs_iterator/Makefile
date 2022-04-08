ccflags-y := -std=gnu99 -DENABLE_DEBUG
proc_fs_iterator-objs := main.o fops.o
obj-m := proc_fs_iterator.o


KERNELDIR ?= /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

modules:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

modules_install:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules_install

clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean
	rm -rf *.o *.ko *.mod.c .tmp_versions *.mod modules.order *.symvers

.PHONY: modules modules_install clean


CFLAGS_main.o := -DDEBUG
CFLAGS_fops.o := -DDEBUG

