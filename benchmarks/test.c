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

void simplef(){
	puts("Donald- you are threaded\n");
	rpthread_yield();
	puts("We're back\n");
	//rpthread_exit(NULL);

}

void simplef2(){
	puts("Bob- you are threaded\n");
	rpthread_yield();
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

ucontext_t* mainContext=NULL;
rpthread_t mainThread=NULL;


int main(int argc, char **argv) {

	/* Implement HERE */

	if(mainThread==NULL){
		rpthread_create(&mainThread,NULL,main,NULL);
		ucontext_t* temp=tempFunction();
		setcontext(temp);
	}

	rpthread_t thread, thread2;
	rpthread_create(&thread, NULL, simplef, NULL);
	printf("Here in main after Thread 1\n");
	rpthread_create(&thread2 ,NULL, simplef2, NULL);
	int* val;
	rpthread_join(thread2, &val);
	printf("JOIN RETURN VALUE THINGY: %d\n", *val);
	rpthread_exit(NULL);
	return 0;
}
