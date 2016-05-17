/* 
 *      CS356 Operating System Project 2: set_sched_zygote
 *	This program sets the scheduler of all descendants of process 
 *	zygote to SCHED_RR and sets their priority properly.
 *           anthor: Ma Yesheng 5140209064
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sched.h>

#define BUF_SIZE 100
#define ZYGOTE_PROCESS "main"
#define MYTEST_PROCESS "my_test"
#define TEST_PROCESS "est.processtest"

/* prinfo: structure to store task info */
struct prinfo {
    pid_t parent_pid;
    pid_t pid;
    pid_t first_child_pid;
    pid_t next_sibling_pid;
    long state;
    long uid;
    char comm[16];
};


inline void print_usage(void)
{
	printf("Usage: specify the priority of test process by greater, equal, smaller\n");
}

int main(int argc, char **argv) 
{
	/* input validity check */
	if (argc != 2) {
		printf("set_sched_zygote: invalid argument number\n");
		print_usage();
		return -1;
	}
	if (strcmp(argv[1], "smaller")
		&& strcmp(argv[1], "equal")
		&& strcmp(argv[1], "greater")) {
		printf("set_test_zygote: invalid priority\n");
		print_usage();
		return -1;
	}
	
	int buf_size = BUF_SIZE;
    struct prinfo *buf;
    buf = malloc(buf_size*sizeof(struct prinfo));
	/* fetch infomation of current running tasks */
    if (syscall(356, buf, &buf_size) != 0) {
        printf("ERROR: ptree failure\n");
        return -1;
    }
	
	int i, j;
	int test_cnt = 0;
	int target_sched;	/* target scheduler */
	int prev_sched;		/* previous scheduler */
	int curr_sched;		/* scheduler after change */
	struct sched_param test_param;
	struct sched_param zygote_param;

	/* set sched_param according to cmd line option */
	if (strcmp(argv[1], "smaller") == 0) {
		test_param.sched_priority = 50;
		zygote_param.sched_priority = 99;
	} else if (strcmp(argv[1], "equal") == 0) {
		test_param.sched_priority = 99;
		zygote_param.sched_priority = 99;
	} else if (strcmp(argv[1], "greater") == 0) {
		test_param.sched_priority = 99;
		zygote_param.sched_priority = 50;
	}

	
	/* this part deals with descendants of zygote */
	
	/* get the index for process zygote */
	int main_index = -1;
	for (i = 0; i<buf_size && main_index==-1; ++i)
		if (strcmp(buf[i].comm, "main") == 0)
			main_index = i;
	if (main_index == -1) {
		printf("set_sched_zygote: process zygote is not found\n");
		return -1;
	}

	/*
	 *	traverse through processes following zygote and if the process is a 
	 *  descendant of zygote then we change it to proper priority and scheduler.
	 */
	for (i = main_index+1; i < buf_size; ++i) {
		/* test whether this process is descendant of previous processes */
		for (j = i-1; j >= main_index; --j) {
			if (buf[i].parent_pid == buf[j].pid) {
				sched_setscheduler(buf[i].pid, SCHED_RR, &zygote_param);
				printf("zygote descendant process %s is set to %d\n",
					buf[i].comm, sched_getscheduler(buf[i].pid));
				break;
			}
		}
		if (j == main_index-1)
			break;
	}

	for (i = 0; i < buf_size; ++i) {
		if (strcmp(buf[i].comm, MYTEST_PROCESS) == 0
			|| strcmp(buf[i].comm, TEST_PROCESS) == 0) {
			sched_setscheduler(buf[i].pid, SCHED_RR, &test_param);
			printf("test process %s is set to %d\n",
				buf[i].comm, sched_getscheduler(buf[i].pid));
		}
	}

    free(buf);
    return 0;
}

