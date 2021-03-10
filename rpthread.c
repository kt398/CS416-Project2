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
#define TIME 4

#define STACK_SIZE SIGSTKSZ

#ifndef MLFQ
	static int sched = 0;
#else 
	static int sched = 1;
#endif


Node* head = NULL;
Node* tail = NULL;
Node* blocked = NULL;

ucontext_t* schedContext=NULL;
ucontext_t* exitContext=NULL;
ucontext_t* mainContext=NULL;
tcb* mainBlock=NULL;
struct itimerval timer;

//functions
static void schedule();
static void sched_rr();
void enqueue();
void printList();
Node* findContext(rpthread_t*, Node*);
Node* dequeueBlocked(rpthread_t*, Node*);
ucontext_t* initializeContext();
Node* dequeue(rpthread_t*, Node*);
void enqueueBlocked(Node*);
void time_handler();

ucontext_t* tempFunction(){
	return head->tcb->context;
}

// YOUR CODE HERE

/* create a new thread */
int rpthread_create(rpthread_t * thread, pthread_attr_t * attr, 
                      void *(*function)(void*), void * arg) {

	// create Thread Control Block
	// create and initialize the context of this thread
	// allocate space of stack for this thread to run
	// after everything is all set, push this thread int
	// printf("CREATE\n");
	// YOUR CODE HERE



	if(exitContext==NULL){
		exitContext = initializeContext();
		makecontext(exitContext,(void*)(rpthread_exit),0);
	}

	//Initialize thread control block
	if(mainContext==NULL&&head==NULL){
		// printf("Inside create first if\n");
		rpthread_t mainThread;
		// if (setcontext(&cctx) < 0){
		// 	perror("set current context");
		// 	exit(1);
		// }
		mainBlock=malloc(sizeof(tcb));
		mainBlock->id=malloc(sizeof(rpthread_t));
		mainBlock->id=&mainThread;
		mainBlock->status= READY;
		mainBlock->temp=malloc(5);
		mainBlock->temp="main";
		mainBlock->priority=0;
		mainBlock->parent=malloc(sizeof(rpthread_t));
		mainBlock->parent=NULL;
    	mainBlock->context = malloc(sizeof(ucontext_t));
        if(getcontext(mainBlock->context)<0){
            perror("getcontext error");
            exit(1);
        }		
		mainBlock->context->uc_link=exitContext;
		Node* mainNode = malloc(sizeof(Node));
		mainNode->tcb = mainBlock;
		mainNode->next = NULL;
		enqueue(mainNode);

		// Use sigaction to register signal handler
		struct sigaction sa;
		memset (&sa, 0, sizeof (sa));
		sa.sa_handler = &time_handler;
		sigaction (SIGPROF, &sa, NULL);

		timer.it_interval.tv_usec = 0; 
		timer.it_interval.tv_sec = 0;
		timer.it_value.tv_usec = TIMESLICE;
		timer.it_value.tv_sec = 0;
		// Set the timer up (start the timer)
		setitimer(ITIMER_PROF, &timer, NULL);
	}
	tcb* block = malloc(sizeof(tcb));
	block->id = malloc(sizeof(rpthread_t));
	block->id = thread;
	block->status= SCHEDULED;

	block->priority = 0;
	block->parent = malloc(sizeof(rpthread_t));
	block->parent = NULL;
	
	//initialize scheduler context 
	if(schedContext==NULL){
		schedContext = initializeContext();
		makecontext(schedContext,schedule,0);
	}

	//initialize exit context

	block->context=initializeContext();
	block->context->uc_link = exitContext;
	makecontext(block->context,(void*)function,1,arg);
	block->temp=malloc(8);
	block->temp="notmain";
	Node* node = malloc(sizeof(Node));
	node->tcb = block;
	node->next = NULL;
	enqueue(node);
	//setcontext(head->tcb->context);
	return 0;
};

void time_handler(){
	head->tcb->status = SCHEDULED;
	swapcontext(head->tcb->context, schedContext);
}

ucontext_t* initializeContext(){
	ucontext_t* context=malloc(sizeof(ucontext_t));
	void* stack=malloc(STACK_SIZE);
	if(stack == NULL){
		perror("Failed to allocate stack");
		exit(1);
	}
	if(getcontext(context) < 0){
		perror("getcontext error");
		exit(1);
	}
	context->uc_link=NULL;
	context->uc_stack.ss_sp=stack;
	context->uc_stack.ss_size=STACK_SIZE;
	context->uc_stack.ss_flags=0;
	return context;
}

void printList(){
	Node* temp=head;
	printf("Head List:\n");
	while(temp!=NULL){
		printf("%s,",temp->tcb->temp);
		temp=temp->next;
	}
	printf("\n");
	printf("Blocked List:\n");
	temp=blocked;
	while(temp!=NULL){
		printf("%s,",temp->tcb->temp);
		temp=temp->next;
	}
	printf("\n");
}

/* give CPU possession to other user-level threads voluntarily */
int rpthread_yield() {
	// change thread state from Running to Ready
	// save context of this thread to its thread control block
	// switch from thread context to scheduler co*ntext

	// YOUR CODE HERE

	//save context then switch to scheduler context
	head->tcb->status=SCHEDULED;
	swapcontext(head->tcb->context, schedContext);
	return 0;
};

/* terminate a thread */
void rpthread_exit(void *value_ptr){
	// Deallocated any dynamic memory created when starting this thread
	printf("inside exit\n");
	// YOUR CODE HERE

	
	// printf("status in exit: %d\n",head->tcb->status);
	head->tcb->status=KILL;
	// printf("status in exit after change: %d\n",head->tcb->status);

	// store value_ptr in parent's tcb
	if(head->tcb->parent!=NULL){
		// printf("Parent not null\n");
		Node* par = dequeueBlocked(head->tcb->parent, blocked);
		
		//Set to SCHEDULED and enqueue back to runqueue
		par->tcb->status = SCHEDULED;
		// printf("test\n");
		enqueue(par);
		if(value_ptr!=NULL){
			par->tcb->val= value_ptr;
		}
	}
	schedule();
}

Node* findContext(rpthread_t* key, Node* list){
	Node* temp = list;
	while (temp!=NULL){
		if(temp->tcb->id==key){
			return temp;
		}
		temp=temp->next;
	}
	return NULL;
}


Node* dequeue(rpthread_t* key, Node* list){
	Node* temp = list;
	Node* prev = NULL;
	
	while (temp!=NULL){
		if(temp->tcb->id==key){
			if(prev == NULL){
				head = temp->next;
				temp->tcb->status=READY;
			}else{
				prev->next = temp->next;
				head = prev;
			}
			temp->next = NULL;
			return temp;
		}
		prev = temp;
		temp = temp->next;
	}
	return NULL;
}

Node* dequeueBlocked(rpthread_t* key, Node* list){
	Node* temp = list;
	Node* prev = NULL;
	
	while (temp!=NULL){
		if(temp->tcb->id==key){
			if(prev == NULL){
				blocked = temp->next;
			}else{
				prev->next = temp->next;
				blocked = prev;
			}
			temp->next = NULL;
			return temp;
		}
		prev = temp;
		temp = temp->next;
	}
	return NULL;
}

void enqueueBlocked(Node* toBlock){
	toBlock->next=NULL;
	if(blocked==NULL){
		blocked=toBlock;
	}
	else{
		Node* temp=blocked;
		while(temp->next!=NULL){
			temp=temp->next;
		}
		temp->next=toBlock;

	}
}


/* Wait for thread termination */
int rpthread_join(rpthread_t thread, void **value_ptr) {
	// wait for a specific thread to terminate
	// de-allocate any dynamic memory created by the joining thread
  
	// YOUR CODE HERE
	Node* ptr = head->next;
	while(ptr!=NULL){
		// printf("%d\n", (ptr->tcb->id));
		if(*(ptr->tcb->id) == thread){
			ptr->tcb->parent = head->tcb->id;
			head->tcb->status = BLOCKED;
			// Node* toBlock=dequeue(head->tcb->id,head);
			// enqueueBlocked(toBlock);
			// printList();
			break;
		}
		ptr = ptr->next;
	}
	// printf("inside join, before swap context\n");

	// printList();
	// printf("End Print List\n");
	swapcontext(head->tcb->context, schedContext);
	if(value_ptr!=NULL){
		*value_ptr = head->tcb->val;
	}
	return 0;
};

/* initialize the mutex lock */
int rpthread_mutex_init(rpthread_mutex_t *mutex, 
                          const pthread_mutexattr_t *mutexattr) {
	//initialize data structures for this mutex	
	// YOUR CODE HERE
	mutex=malloc(sizeof(rpthread_mutex_t));
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
	// printf("Inside schedule\n");
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
	int status=head->tcb->status;
	if(status==KILL){ //coming from exit
		// printf("KILL\n");
		Node* temp=head;
		head=head->next;
		// free(temp->tcb->id);
		// free(temp->tcb->parent);
		free(temp->tcb->context->uc_stack.ss_sp);
		free(temp->tcb->context);
		free(temp->tcb);
		free(temp);
	}
	else if(status==SCHEDULED){ //coming from yield
		// printf("Yield?\n");
		if(head->next!=NULL){
			Node* temp = head;
			head = head->next;
			temp->next = NULL;
			tail->next = temp;
			tail = tail->next;
		}
	}
	else if(status == BLOCKED){
		Node* temp=head;
		head=head->next;
		enqueueBlocked(temp);
	}
	if(head!= NULL){
		head->tcb->status=READY;
		timer.it_interval.tv_usec = 0;
		timer.it_value.tv_usec = TIMESLICE;
		setitimer(ITIMER_PROF, &timer, NULL);
		setcontext(head->tcb->context);
	}
	else{
		printf("End of program?\n");
		free(schedContext->uc_stack.ss_sp);
		free(schedContext);
		free(exitContext->uc_stack.ss_sp);
		free(exitContext);
		printf("Freed Everything?\n");

	}

	//Otherwise, we switch to the next thread in the runqueue
	// head->tcb->status = SCHEDULED;
	// Node* temp = head;
	// tail->next = temp;
	// tail = tail->next;
	// head = head->next;
	// temp->next=NULL;
	// head->tcb->status = READY;

	// swapcontext(temp->tcb->context, head->tcb->context);
}

/* Preemptive MLFQ scheduling algorithm */
static void sched_mlfq() {
	// Your own implementation of MLFQ
	// (feel free to modify arguments and return types)

	// YOUR CODE HERE
}

// Feel free to add any other functions you need

// YOUR CODE HERE
void enqueue(Node* node){
	if(head == NULL){
		head = node;
		tail = head;
		return;
	}
	#ifndef MLFQ
		// RR - Enqueue into end of runqueue
		tail->next = node;
		tail = tail->next;
		tail->next = NULL;
	#else 
		// MLFQ - Enqueue to the topmost queue

	#endif
}
