#ifndef _MAIN_H
#define _MAIN_H

#include <linux/list.h>			/* double linked list support */

#define MODULE_NAME		"orange"

struct data_block {
	char data;
	struct list_head data_list;
};

struct orange_dev {
	int block_counter;		/* record how many blocks now in the list */
	struct mutex mutex;
	struct cdev cdev;
	struct list_head list_entry;	/* list of storage blocks */
};


#endif


