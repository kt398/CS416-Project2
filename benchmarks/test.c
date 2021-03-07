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
	//rpthread_yield();
	puts("We're back\n");
	//rpthread_exit(NULL);

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
		rpthread_create(mainThread,NULL,main,NULL);
		ucontext_t* temp=tempFunction();
		setcontext(temp);
	}
	

	// if(mainContext==NULL){
	// 	rpthread_t main;
	// 	mainContext=malloc(sizeof(ucontext_t));
	// 	void* schedStack=malloc(STACK_SIZE);
	// 	if(schedStack == NULL){
	// 		perror("Failed to allocate stack");
	// 		exit(1);
	// 	}
	// 	if(getcontext(mainContext) < 0){
	// 		perror("getcontext");
	// 		exit(1);
	// 	}
	// 	tcb block;
	// 	block.context=malloc(sizeof(ucontext_t));
	// 	block.id=main;
	// 	block.status=0;
	// 	block.priority=0;
	// 	block.parent=NULL;
	// 	printf("randomprint2\n");
	// 	if(getcontext(block.context) < 0){
	// 		printf("what\n");
	// 		perror("getcontext");
	// 		exit(1);
	// 	}
	// 	printf("middle of if\n");
	// 	block.stack=malloc(STACK_SIZE);
	// 	if (block.stack == NULL){
	// 		perror("Failed to allocate stack");
	// 		exit(1);
	// 	}
	// 	mainContext->uc_link=NULL;
	// 	mainContext->uc_stack.ss_sp=block.stack;
	// 	mainContext->uc_stack.ss_size=STACK_SIZE;
	// 	mainContext->uc_stack.ss_flags=0;
	// 	makecontext(mainContext,main,0);
	// 	printf("end of if\n");
	// 	enqueue(block);
	// 	printf("end\n");
	// }
	rpthread_t thread, thread2;
	printf("after if\n");
	rpthread_create(thread, NULL, simplef, NULL);
	printf("Here in main after Thread 1\n");
	rpthread_create(thread2 ,NULL, simplef, NULL);
	rpthread_exit(NULL);
	return 0;
}
