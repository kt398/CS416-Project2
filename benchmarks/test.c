#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include "../rpthread.h"
#include <signal.h>
#include "ucontext.h"
#include <sys/syscall.h>
#include <sys/types.h>
#include <stdlib.h>
#define STACK_SIZE SIGSTKSZ

int x = 0;
void simplef(){
	puts("Donald- you are threaded\n");
	// rpthread_yield();
	puts("We're back\n");
	//rpthread_exit(NULL);

}

void incrementTest(){
	while(1){
		x++;
		// printf("X VALUE: %d\n", x);
	}
}

void simplef2(){
	puts("Bob- you are threaded\n");
	// rpthread_yield();
	puts("We're back2\n");
	int* val = malloc(sizeof(int));
	*val = 5;
	rpthread_exit(val);
}
/* A scratch program template on which to call and
 * test rpthread library functions as you implement
 * them.
 *
 * You can modify and use this program as much as possible.
 * This will not be graded.
 */

rpthread_t mainThread=NULL;


int main(int argc, char **argv) {

	/* Implement HERE */


	rpthread_t thread, thread2;
	rpthread_create(&thread, NULL, incrementTest, NULL);
	printf("Here in main after Thread 1\n");
	rpthread_create(&thread2 ,NULL, incrementTest, NULL);
	int* val;
	rpthread_join(thread2, &val);
	printf("JOIN RETURN VALUE THINGY: %d\n", *val);
	rpthread_exit(NULL);
	return 0;
}
