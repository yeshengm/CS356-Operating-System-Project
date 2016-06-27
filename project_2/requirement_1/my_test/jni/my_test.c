/* 
 *      CS356 Operating System Project 2: my_test
 *	This program just functions as a test function which
 *  adds number from 1 to N where N is an input parameter
 *	and calculate the time it consumes.
 *           anthor: Ma Yesheng 5140209064
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>


int main(int argc, char **argv)
{
	int N = 100000000;
	clock_t begin, end;
	begin = clock();
	int sum = 0;
	int i;
	int j;
	for (j = 0; j <= 5; ++j)
		for (i = 0; i <= N; ++i)
			sum += i;
	end = clock();
	printf("my_test over, time consumed %d ms\n", 
		(int)((double)(end-begin)/CLOCKS_PER_SEC*1000));
	return 0;
}
