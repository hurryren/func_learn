#include <linux/init.h>
#include <linux/module.h>

MODULE_LICENSE("Dual BSD/GPL");

static int orange_init(void){
	printk(KERN_ALERT "hello, world\n");
	return 0;
}

static void orange_exit(void){
	printk(KERN_ALERT "Goodbye, cruel world\n");
}

module_init(orange_init);
module_exit(orange_exit);


