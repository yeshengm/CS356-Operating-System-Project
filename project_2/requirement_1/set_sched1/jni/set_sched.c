/* 
 *      CS356 Operating System Project 2: set_sched
 *	this program changes scheduler of test apps according to 
 *	the command line argument of the program.
 *	Usage: ./set_sched -t FIFO/RR
 *		   ./set_sched -tz			for problem 1
 *		   ./set_sched -m			for problem 2
 *	           anthor: Ma Yesheng 5140209064
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
	long prio;
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
	printf("Usage: ./set_test_sched -t/-m scheduler\n"
		"Scheduler should be one of NORMAL, FIFO or RR\n");
}


int main(int argc, char **argv) 
{
	/* input validity check */
	/*
	if (argc != 3 && argc != 4) {
		printf("OSPrj2: invalid argument number\n");
		print_usage();
		return -1;
	}
	*/
	if (strcmp(argv[1], "-m") && strcmp(argv[1], "-t") && strcmp(argv[1], "-tz")) {
		printf("OSPrj2: invalid test type\n");
	}

	int buf_size = BUF_SIZE;
    struct prinfo *buf;
    buf = (struct prinfo *) malloc(buf_size*sizeof(struct prinfo));
	/* fetch infomation of current running tasks */
    if (syscall(356, buf, &buf_size) != 0) {
        printf("ERROR: ptree failure\n");
        return -1;
    }
	
	// change scheduler of test processes
	if (strcmp(argv[1], "-t") == 0) {
		int i;
		int target_sched;	/* target scheduler */
		int prev_sched;		/* previous scheduler */
		int curr_sched;		/* scheduler after change */
		int max_priority;
		struct sched_param param;

		printf("OSPrj2: change only priority of test process\n");
		target_sched = get_sched_value(argv[2]);
		max_priority = sched_get_priority_max(target_sched);
		param.sched_priority = max_priority;
		param.sched_priority = atoi(argv[3]);


		for (i = 0; i < buf_size; ++i) {	
			if (strcmp(buf[i].comm, TEST_PROCESS)==0 
					|| strcmp(buf[i].comm, MYTEST_PROCESS)==0) {
				prev_sched = sched_getscheduler(buf[i].pid);
				sched_setscheduler(buf[i].pid, target_sched, &param);
				curr_sched = sched_getscheduler(buf[i].pid);
				printf("Process %s: %s -> %s\n", buf[i].comm, 
							get_sched_str(prev_sched), get_sched_str(curr_sched));
			}
		}
	}


	// change the descendants of zygote
	if (strcmp(argv[1], "-tz") == 0) {
		int i, j;
	    struct sched_param test_param;
	    struct sched_param zygote_param;
		int target_sched;
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

		target_sched = get_sched_value(argv[2]);
		zygote_param.sched_priority = 1; 
	    /*
	     *	traverse through processes following zygote and if the process is a 
	     *  descendant of zygote then we change it to proper priority and scheduler.
	     */
	    for (i = main_index+1; i < buf_size; ++i) {
		    /* test whether this process is descendant of previous processes */
		    for (j = i-1; j >= main_index; --j) {
			    if (buf[i].parent_pid == buf[j].pid) {
				    sched_setscheduler(buf[i].pid, SCHED_RR, &zygote_param);
				    printf("zygote descendant process %s is set to %s\n",
					    buf[i].comm, get_sched_str(sched_getscheduler(buf[i].pid)));
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
			    printf("test process %s is set to %s\n",
				    buf[i].comm, get_sched_str(sched_getscheduler(buf[i].pid)));
		    }
	    }
	}

	// set scheduler for modified scheduler
	if (strcmp(argv[1], "-m") == 0) {
		int i;
		int prev_sched;
		int curr_sched;
		struct sched_param param;

		param.sched_priority = atoi(argv[2]);
		for (i = 0; i < buf_size; ++i) {	
			if (strcmp(buf[i].comm, MYTEST_PROCESS)==0||strcmp
					(buf[i].comm,TEST_PROCESS)==0) {
				prev_sched = sched_getscheduler(buf[i].pid);
				sched_setscheduler(buf[i].pid, SCHED_RR, &param);
				curr_sched = sched_getscheduler(buf[i].pid);
				printf("Process %s: %s -> %s\n", buf[i].comm, 
						get_sched_str(prev_sched), get_sched_str(curr_sched));
			}
		}
	}

    free(buf);
    return 0;
}

