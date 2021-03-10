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

int x = 100;
rpthread_mutex_t mutex;
void simplef(){
	puts("Donald- you are threaded\n");
	// rpthread_yield();
	puts("We're back\n");
	//rpthread_exit(NULL);

}

void increment(){
	rpthread_mutex_lock(&mutex);
	for(int i=0; i<100000; i++){
		printf("%d\n", i);
	}
	rpthread_mutex_unlock(&mutex);
}

void incrementTest(){
	int n = x;
	n+=100;
	x = n;
	printf("X VALUE 1: %d\n", x);
}

void incrementTest2(){
	int n = x;
	n+=50;
	x = n;
	printf("X VALUE 2: %d\n", x);
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
	rpthread_mutex_init(&mutex, NULL);
	rpthread_create(&thread, NULL, increment, NULL);
	printf("Here in main after Thread 1\n");
	rpthread_create(&thread2 ,NULL, increment, NULL);
	int* val;
	// rpthread_join(thread2, &val);
	printf("JOIN RETURN VALUE THINGY: %d\n", *val);
	rpthread_exit(NULL);
	return 0;
}
