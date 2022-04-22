
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>

static int hello3_data __initdata = 3;

static int __init  hello_init(void) {
	pr_info("hello world 1.\n");
	return 0;
}

static void __exit hello_exit(void){
	pr_info("Goodbye world 1.\n");
}

module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE("GPL");
