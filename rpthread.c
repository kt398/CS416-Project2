// File:	rpthread.c

// List all group member's name:
// username of iLab:
// iLab Server:

#include "rpthread.h"

// INITAILIZE ALL YOUR VARIABLES HERE
#define READY 0
#define SCHEDULED 1
#define BLOCKED 2
#define STACK_SIZE SIGSTKSZ

Node* head;
Node* tail;

ucontext_t* sched=NULL;

// YOUR CODE HERE


/* create a new thread */
int rpthread_create(rpthread_t * thread, pthread_attr_t * attr, 
                      void *(*function)(void*), void * arg) {
       // create Thread Control Block
       // create and initialize the context of this thread
       // allocate space of stack for this thread to run
       // after everything is all set, push this thread int
       // YOUR CODE HERE
   		printf("hello12345\n");
		printf("doesn't works???\n");

		tcb block;
		block.id=thread;
		block.status=READY;
		block.priority=0;
		if(getcontext(block.context) < 0){
			perror("getcontext");
			exit(1);
		}


		block.stack=malloc(STACK_SIZE);
		if (block.stack == NULL){
			perror("Failed to allocate stack");
			exit(1);
		}
   		printf("hello\n");

		if(sched==NULL){
			printf("NULL\n");
			sched=malloc(sizeof(ucontext_t));
			void* schedStack=malloc(STACK_SIZE);
			if(schedStack == NULL){
				perror("Failed to allocate stack");
				exit(1);
			}
			if(getcontext(sched) < 0){
				perror("getcontext");
				exit(1);
			}
			sched->uc_link=NULL;//Change when adding next thread
			sched->uc_stack.ss_sp=schedStack;
			sched->uc_stack.ss_size=STACK_SIZE;
			sched->uc_stack.ss_flags=0;
			makecontext(sched,schedule,0);
		}
		//Point uc_link to scheduler
		block.context->uc_link=sched;//Change when adding next thread
		block.context->uc_stack.ss_sp=block.stack;
		block.context->uc_stack.ss_size=STACK_SIZE;
		block.context->uc_stack.ss_flags=0;

		makecontext(block.context,function,0);
		setcontext(block.context);
		printf("works???\n");
	return 0;
};

/* give CPU possession to other user-level threads voluntarily */
int rpthread_yield() {
	head->tcb->status=SCHEDULED;
	// change thread state from Running to Ready
	// save context of this thread to its thread control block
	// wwitch from thread context to scheduler context

	// YOUR CODE HERE
	return 0;
};

/* terminate a thread */
void rpthread_exit(void *value_ptr) {
	free(head->tcb->stack);
	// Deallocated any dynamic memory created when starting this thread

	// YOUR CODE HERE
};


/* Wait for thread termination */
int rpthread_join(rpthread_t thread, void **value_ptr) {
	
	// wait for a specific thread to terminate
	// de-allocate any dynamic memory created by the joining thread
  
	// YOUR CODE HERE
	return 0;
};

/* initialize the mutex lock */
int rpthread_mutex_init(rpthread_mutex_t *mutex, 
                          const pthread_mutexattr_t *mutexattr) {
	//initialize data structures for this mutex

	// YOUR CODE HERE
	return 0;
};

/* aquire the mutex lock */
int rpthread_mutex_lock(rpthread_mutex_t *mutex) {
        // use the built-in test-and-set atomic function to test the mutex
        // if the mutex is acquired successfully, enter the critical section
        // if acquiring mutex fails, push current thread into block list and //  
        // context switch to the scheduler thread

        // YOUR CODE HERE
        return 0;
};

/* release the mutex lock */
int rpthread_mutex_unlock(rpthread_mutex_t *mutex) {
	// Release mutex and make it available again. 
	// Put threads in block list to run queue 
	// so that they could compete for mutex later.

	// YOUR CODE HERE
	return 0;
};


/* destroy the mutex */
int rpthread_mutex_destroy(rpthread_mutex_t *mutex) {
	// Deallocate dynamic memory created in rpthread_mutex_init

	return 0;
};

/* scheduler */
static void schedule() {
	puts("Schedule\n");
	// Every time when timer interrup happens, your thread library 
	// should be contexted switched from thread context to this 
	// schedule function

	// Invoke different actual scheduling algorithms
	// according to policy (RR or MLFQ)

	// if (sched == RR)
	//		sched_rr();
	// else if (sched == MLFQ)
	// 		sched_mlfq();

	// YOUR CODE HERE

// schedule policy
#ifndef MLFQ
	// Choose RR
     // CODE 1
#else 
	// Choose MLFQ
     // CODE 2
#endif

}

/* Round Robin (RR) scheduling algorithm */
static void sched_rr() {
	Node* temp= head;
	tail->next=temp;
	temp->next=NULL;
	head=head->next;
	swapcontext(temp->tcb->context,head->tcb->context);
	// Your own implementation of RR
	// (feel free to modify arguments and return types)

	// YOUR CODE HERE
}

/* Preemptive MLFQ scheduling algorithm */
static void sched_mlfq() {
	// Your own implementation of MLFQ
	// (feel free to modify arguments and return types)

	// YOUR CODE HERE
}

// Feel free to add any other functions you need

// YOUR CODE HERE

