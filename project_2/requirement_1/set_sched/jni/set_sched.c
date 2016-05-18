/* 
 *      CS356 Operating System Project 2: sched_fifo
 *	This program changes current scheduler of the test
 * 	process from SCHED_NORMAL to SCHED_FIFO and priority
 *	set to max priority.
 *           anthor: Ma Yesheng 5140209064
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sched.h>
#include <unistd.h>

#define BUF_SIZE 100
#define TEST_PROCESS "est.processtest"
#define MYTEST_PROCESS "my_test"

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

/* get_sched_str: print scheduler according to sched number */
char *get_sched_str(int sched)
{
	switch(sched) {
		case 0:
			return "SCHED_NORMAL";
		case 1:
			return "SCHED_FIFO";
		case 2:
			return "SCHED_RR";
		case 3:
			return "SCHED_BATCH";
		case 5:
			return "SCHED_IDLE";
		case 6:
			return "SCHED_DEADLINE";
		default:
			return "ERROR";
	}
}

/* get_sched_value: get the value of scheduler from input string */
int get_sched_value(char sched[])
{
	if (strcmp("NORMAL", sched) == 0)
		return SCHED_NORMAL;
	else if (strcmp("FIFO", sched) == 0)
		return SCHED_FIFO;
	else if (strcmp("RR", sched) == 0)
		return SCHED_RR;
	else
		return -1;
}

inline void print_usage()
{
	printf("Usage: ./set_test_sched scheduler\n"
		"Scheduler should be one of NORMAL, FIFO or RR\n");
}


int main(int argc, char **argv) 
{
	/* input validity check */
	if (argc != 2) {
		printf("set_test_sched: invalid argument number\n");
		print_usage();
		return -1;
	}
	if (strcmp(argv[1], "NORMAL")
		&& strcmp(argv[1], "RR")
		&& strcmp(argv[1], "FIFO")) {
		printf("set_test_sched: invalid scheduler name\n");
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
	
	int i;
	int test_cnt = 0;
	int target_sched;	/* target scheduler */
	int prev_sched;		/* previous scheduler */
	int curr_sched;		/* scheduler after change */
	int max_priority; 
	struct sched_param param;

	target_sched = get_sched_value(argv[1]);
	max_priority = sched_get_priority_max(target_sched);
	param.sched_priority = max_priority;
	
	/*
	 * traverse through all processes and if the process is
	 * the test process, we change the scheduler of it from
	 * SCHED_NORMAL to SCHED_FIFO.
	 */
	for (i = 0; i < buf_size; ++i) {
		if (strcmp(buf[i].comm, TEST_PROCESS)==0 || strcmp(buf[i].comm, MYTEST_PROCESS)==0) {
			prev_sched = sched_getscheduler(buf[i].pid);
			sched_setscheduler(buf[i].pid, target_sched, &param);
			curr_sched = sched_getscheduler(buf[i].pid);
		}
	}

    free(buf);
    return 0;
}

