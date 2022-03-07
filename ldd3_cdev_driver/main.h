#ifndef _MAIN_H
#define _MAIN_H

#include <linux/list.h>			/* double linked list support */

#define MODULE_NAME		"orange"
#define ORANGE_NR_DEVS		3
#define ORANGE_BLOCK_SIZE	PAGE_SIZE		/* one page per block */

struct orange_block {
	loff_t offset;
	char data[ORANGE_BLOCK_SIZE];
	struct list_head block_list;
};

struct orange_dev {
	int block_counter;		/* record how many blocks now in the list */
	struct mutex mutex;
	struct cdev cdev;
	struct list_head block_list;	/* list of storage blocks */
};


#endif


