#ifndef _MAIN_H
#define _MAIN_H

#define MODULE_NAME	"pipe_advanced_sleep"
#define BUFF_SIZE 	(1 << 3)  /* b bytes only */
#define PIPE_DEV_NR 	3

struct pipe_dev {
	struct cdev cdev;
	struct mutex mutex;
	wait_queue_head_t rd_queue;
	wait_queue_head_t wr_queue;
	int buff_len;
	char buff[BUFF_SIZE];
};

#endif
