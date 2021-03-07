// File:	rpthread.c

// List all group member's name:
// username of iLab:
// iLab Server:

#include "rpthread.h"

// INITAILIZE ALL YOUR VARIABLES HERE
#define READY 0
#define SCHEDULED 1
#define BLOCKED 2
#define KILL 3
#define STACK_SIZE SIGSTKSZ

#ifndef MLFQ
	static int sched = 0;
#else 
	static int sched = 1;
#endif


Node* head = NULL;
Node* tail = NULL;

ucontext_t* schedContext=NULL;
ucontext_t* exitContext=NULL;

static void schedule();
static void sched_rr();
void enqueue();
void printList();



ucontext_t* tempFunction(){
	return head->tcb->context;
}

Node* findContext(rpthread_t* key);

// YOUR CODE HERE


/* create a new thread */
int rpthread_create(rpthread_t * thread, pthread_attr_t * attr, 
                      void *(*function)(void*), void * arg) {
	// create Thread Control Block
	// create and initialize the context of this thread
	// allocate space of stack for this thread to run
	// after everything is all set, push this thread int

	// YOUR CODE HERE
	tcb* block=malloc(sizeof(block));

	block->id = thread;
	block->status= SCHEDULED;
	block->priority = 0;
	block->parent = NULL;
	block->context=malloc(sizeof(ucontext_t));

	if(getcontext(block->context) < 0){
		perror("getcontext");
		exit(1);
	}

	block->stack=malloc(STACK_SIZE);
	if (block->stack == NULL){
		perror("Failed to allocate stack");
		exit(1);
	}

	//initialize scheduler context 
	if(schedContext==NULL){
		schedContext=malloc(sizeof(ucontext_t));
		void* schedStack=malloc(STACK_SIZE);
		if(schedStack == NULL){
			perror("Failed to allocate stack");
			exit(1);
		}
		if(getcontext(schedContext) < 0){
			perror("getcontext");
			exit(1);
		}
		schedContext->uc_link=NULL;
		schedContext->uc_stack.ss_sp=schedStack;
		schedContext->uc_stack.ss_size=STACK_SIZE;
		schedContext->uc_stack.ss_flags=0;
		makecontext(schedContext,schedule,0);
	}


	//initialize exit context
	if(exitContext==NULL){
		exitContext=malloc(sizeof(ucontext_t));
		void* exitStack=malloc(STACK_SIZE);
		if(exitStack == NULL){
			perror("Failed to allocate stack");
			exit(1);
		}
		if(getcontext(exitContext) < 0){
			perror("getcontext");
			exit(1);
		}
		exitContext->uc_link=NULL;
		exitContext->uc_stack.ss_sp=exitStack;
		exitContext->uc_stack.ss_size=STACK_SIZE;
		exitContext->uc_stack.ss_flags=0;
		makecontext(exitContext,rpthread_exit,0);
	}



	//Point uc_link to scheduler 
	block->context->uc_link=exitContext;
	block->context->uc_stack.ss_sp=block->stack;
	block->context->uc_stack.ss_size=STACK_SIZE;
	block->context->uc_stack.ss_flags=0;
	printf("end of create?\n");
	makecontext(block->context,function,0);
	enqueue(block);
	printList();
	//setcontext(head->tcb->context);
	return 0;
};

void printList(){
	Node* temp=head;
	while(temp!=NULL){
		printf("%d\n",temp->tcb->status);
		temp=temp->next;
	}
}
/* give CPU possession to other user-level threads voluntarily */
int rpthread_yield() {
	// change thread state from Running to Ready
	// save context of this thread to its thread control block
	// switch from thread context to scheduler co*ntext

	// YOUR CODE HERE
	head->tcb->status=SCHEDULED;
	//save context then switch to scheduler context
	swapcontext(head->tcb->context, schedContext);
	return 0;
};

/* terminate a thread */
void rpthread_exit(void *value_ptr){
	// Deallocated any dynamic memory created when starting this thread
	printf("inside exit\n");
	// YOUR CODE HERE
	//store value_ptr somewhere
	printf("status in exit: %d\n",head->tcb->status);
	head->tcb->status=KILL;
	printf("status in exit after change: %d\n",head->tcb->status);

	
	// if(value_ptr!=NULL){
	// 	int* value = (int*) malloc(sizeof(int));
	// 	*value = *(int*)value_ptr;
	// }
	// //free dynamic memory and terminate

	// if(head->tcb->parent!=NULL){
	// 	printf("Parent not null\n");
	// 	Node* par=findContext(head->tcb->parent);
	// 	par->tcb->status=SCHEDULED;
	// 	if(value_ptr!=NULL){
	// 		par->tcb->val=value_ptr;
	// 	}
	// }
	schedule();
}


Node* findContext(rpthread_t* key){
	Node* temp=head;
	while (temp!=NULL){
		if(temp->tcb->id==key){
			return temp;
		}
		temp=temp->next;
	}
	return NULL;
}

/* Wait for thread termination */
int rpthread_join(rpthread_t thread, void **value_ptr) {
	// wait for a specific thread to terminate
	// de-allocate any dynamic memory created by the joining thread
  
	// YOUR CODE HERE
	//get value_ptr from stored
	//set retval

	Node* ptr = head->next;
	while(ptr!=NULL){
		if(*(ptr->tcb->id) == thread){
			ptr->tcb->parent = head->tcb->id;
			head->tcb->status = BLOCKED;
			break;
		}
	}

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
	printf("Inside schedule\n");
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
	sched_rr();
#else 
	// Choose MLFQ
     // CODE 2
	sched_mlfq();
#endif

}

/* Round Robin (RR) scheduling algorithm */
static void sched_rr() {
	// Your own implementation of RR
	// (feel free to modify arguments and return types)

	// YOUR CODE HERE
	//No other processes in runqueue so we continue running process
	printf("%d\n",head->tcb->status);
	int status=head->tcb->status;
	
	if(status==KILL){
		printf("KILL\n");
		Node* temp=head;
		head=head->next;
		free(temp->tcb);
		free(temp);
	}
	else if(status==SCHEDULED){

	}

	if(head!= NULL){
		printf("sched_rr\n");
		head->tcb->status=READY;
		setcontext(head->tcb->context);
		return;	
	}
	else{
		printf("here\n");
		exit(0);
	}

	//Otherwise, we switch to the next thread in the runqueue
	head->tcb->status = SCHEDULED;
	Node* temp = head;
	tail->next = temp;
	tail = tail->next;
	head = head->next;
	temp->next=NULL;
	head->tcb->status = READY;
	swapcontext(temp->tcb->context, head->tcb->context);
}

/* Preemptive MLFQ scheduling algorithm */
static void sched_mlfq() {
	// Your own implementation of MLFQ
	// (feel free to modify arguments and return types)

	// YOUR CODE HERE
}

// Feel free to add any other functions you need

// YOUR CODE HERE
void enqueue(tcb* block){
	if(head == NULL){
		head = malloc(sizeof(Node));
		head->tcb = block;
		head->next = NULL;
		tail = head;
		return;
	}
#ifndef MLFQ
	// RR - Enqueue into end of runqueue
	Node* ptr = tail;
	ptr->next = malloc(sizeof(Node));
	ptr->next->tcb = block;
	ptr->next->next = NULL;
	tail = ptr->next;
	
#else 
	// MLFQ - Enqueue to the topmost queue

#endif
}
