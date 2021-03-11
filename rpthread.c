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
#define MUTEX 5

#define STACK_SIZE SIGSTKSZ

#ifndef MLFQ
	static int levels=1;
#else 
	static int levels=4;
#endif

//variables
Node** head = NULL;
Node** tail = NULL;
Node* blocked = NULL;
int threadNum = 0;

Node* mainNode;
ucontext_t* schedContext=NULL;
ucontext_t* exitContext=NULL;
ucontext_t* mainContext=NULL;
tcb* mainBlock=NULL;
struct itimerval timer;
int inMutex=0;//1 if current thread is in the mutex function, 0 otherwise
int isInit=0;//Boolean to check if first time initialization has been completed
int currentLevel;//Current level of the mlfq, if rr then it is always 0

//functions
static void schedule();
static void sched_rr();
static void sched_mlfq();

// YOUR CODE HERE
void firstTimeInit(){
	currentLevel=0;
	head=malloc(levels*sizeof(Node*));
	tail=malloc(levels*sizeof(Node*));
	for(int i=0; i<levels; i++){
		head[i] = NULL;
		tail[i] = NULL;
	}

	//Initializing the exit context
	exitContext = initializeContext();
	makecontext(exitContext,(void*)(rpthread_exit),0);

	//Initialize the context for the main thread
	rpthread_t mainThread;
	mainBlock=malloc(sizeof(tcb));
	mainBlock->id=threadNum;
	mainBlock->status= READY;
	mainBlock->priority=0;
	mainBlock->parent=UINT_MAX;
	mainBlock->context = malloc(sizeof(ucontext_t));
	if(getcontext(mainBlock->context)<0){
		perror("getcontext error");
		exit(1);
	}		
	mainNode = malloc(sizeof(Node));
	mainNode->tcb = mainBlock;
	mainNode->next = NULL;
	enqueue(mainNode,0);

	// Use sigaction to register signal handler
	struct sigaction sa;
	memset (&sa, 0, sizeof (sa));
	sa.sa_handler = &time_handler;
	sigaction (SIGPROF, &sa, NULL);

	//initialize scheduler context 
	if(schedContext==NULL){
		schedContext = initializeContext();
		makecontext(schedContext,schedule,0);
	}
	timer.it_interval.tv_usec = 0; 
	timer.it_interval.tv_sec = 0;
	resetTimer();
	isInit=1;
}

/* create a new thread */
int rpthread_create(rpthread_t * thread, pthread_attr_t * attr, 
                      void *(*function)(void*), void * arg) {

	// create Thread Control Block
	// create and initialize the context of this thread
	// allocate space of stack for this thread to run
	// after everything is all set, push this thread int
	
	// YOUR CODE HERE
	if(isInit==0){
		firstTimeInit();
	}

	tcb* block = malloc(sizeof(tcb));
	block->id = ++threadNum;
	block->status= SCHEDULED;
	block->priority = 0;
	block->parent = UINT_MAX;
	block->context=initializeContext();
	block->context->uc_link = exitContext;
	makecontext(block->context,(void*)function,1,arg);
	*thread = block->id;	
	Node* node = malloc(sizeof(Node));
	node->tcb = block;
	node->next = NULL;
	enqueue(node,0);
	return 0;
};

void time_handler(){
	if(inMutex==1){
		resetTimer();
		return;
	}
	//BIG CHANGES FOR MLFQ
	if(head[currentLevel]->next!=NULL){
		head[currentLevel]->tcb->status = TIME;
		swapcontext(head[currentLevel]->tcb->context, schedContext);
		return;
	}else{
		resetTimer();
		return;
	}
}

void resetTimer(){
	timer.it_interval.tv_usec = 0;
	timer.it_value.tv_usec = TIMESLICE;
	setitimer(ITIMER_PROF, &timer, NULL);
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
	Node* temp;
	for(int i=0; i<levels; i++){
		temp=head[i];
		printf("%d List:\n", i);
		while(temp!=NULL){
			printf("%s,",temp->tcb->temp);
			temp=temp->next;
		}
		printf("\n");
	}
	
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
	// switch from thread context to scheduler context

	// YOUR CODE HERE
	if(head[currentLevel]->next!=NULL){
		head[currentLevel]->tcb->status=SCHEDULED;
		swapcontext(head[currentLevel]->tcb->context, schedContext);
	}else{
		resetTimer();
	}
	return 0;
};

/* terminate a thread */
void rpthread_exit(void *value_ptr){
	// Deallocated any dynamic memory created when starting this thread
	
	// YOUR CODE HERE
	inMutex=1;
	head[currentLevel]->tcb->status=KILL;

	if(head[currentLevel]->tcb->parent!=UINT_MAX){
		Node* par = dequeueBlocked(head[currentLevel]->tcb->parent, blocked);
		
		//Set to SCHEDULED and enqueue back to runqueue
		par->tcb->status = SCHEDULED;
		enqueue(par,par->tcb->priority);
		
		// store value_ptr in parent's tcb
		if(value_ptr!=NULL){
			par->tcb->val= value_ptr;
		}
	}
	inMutex=0;
	schedule();
}

Node* dequeueBlocked(rpthread_t key, Node* list){
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
	if(blocked==NULL){
		blocked = toBlock;
	}else{
		toBlock->next = blocked;
		blocked = toBlock;
	}
}

/* Wait for thread termination */
int rpthread_join(rpthread_t thread, void **value_ptr) {
	// wait for a specific thread to terminate
	// de-allocate any dynamic memory created by the joining thread
	
	// YOUR CODE HERE
	inMutex=1;
	for(int i=0;i<levels;i++){
		Node* ptr = head[i];
		while(ptr!=NULL){
			if(ptr->tcb->id == thread && ptr!=head[currentLevel]){
				ptr->tcb->parent = head[currentLevel]->tcb->id;
				head[currentLevel]->tcb->status = BLOCKED;
				break;
			}
			ptr = ptr->next;
		}
	}
	inMutex=0;
	swapcontext(head[currentLevel]->tcb->context, schedContext);
	if(value_ptr!=NULL){
		*value_ptr = head[currentLevel]->tcb->val;
	}
	return 0;
};

/* initialize the mutex lock */
int rpthread_mutex_init(rpthread_mutex_t *mutex, 
                          const pthread_mutexattr_t *mutexattr) {
	//initialize data structures for this mutex	
	// YOUR CODE HERE
	mutex->lock = 0;
	// mutex->mutexBlocked = malloc(sizeof(Node));
	mutex->mutexBlocked = NULL;
	return 0;
};

/* aquire the mutex lock */
int rpthread_mutex_lock(rpthread_mutex_t *mutex) {
	// use the built-in test-and-set atomic function to test the mutex
	// if the mutex is acquired successfully, enter the critical section
	// if acquiring mutex fails, push current thread into block list and //  
	// context switch to the scheduler thread

	// YOUR CODE HERE
	inMutex=1;
	while(__sync_lock_test_and_set (&(mutex->lock), 1)){
		Node* temp = head[currentLevel];
		head[currentLevel] = head[currentLevel]->next;
		if(mutex->mutexBlocked == NULL){
			temp->next = NULL;
			mutex->mutexBlocked = temp;
		}else{
			temp->next = mutex->mutexBlocked;
			mutex->mutexBlocked = temp;
		}
		for(int i=0; i<levels; i++){
			if(head[i]!= NULL){
				head[i]->tcb->status=READY;
				currentLevel = i;
				break;
			}
		}
		
		inMutex=0;
		swapcontext(mutex->mutexBlocked->tcb->context, schedContext);
		inMutex=1;
	}

	inMutex=0;
	return 0;
};

/* release the mutex lock */
int rpthread_mutex_unlock(rpthread_mutex_t *mutex) {
	// Release mutex and make it available again. 
	// Put threads in block list to run queue 
	// so that they could compete for mutex later.

	// YOUR CODE HERE
	inMutex=1;
	if(mutex->lock==1){
		Node* ptr = mutex->mutexBlocked;
		while(ptr!=NULL){
			ptr->tcb->status = SCHEDULED;
			int level = ptr->tcb->priority;
			Node* temp = ptr;
			ptr = ptr->next;
			temp->next = NULL;
			enqueue(temp, level);
		}
		mutex->mutexBlocked = NULL;
		__sync_lock_release(&(mutex->lock));
	}
	else{
		printf("Mutex is not locked\n");
	}

	inMutex=0;
	return 0;
};


/* destroy the mutex */
int rpthread_mutex_destroy(rpthread_mutex_t *mutex) {
	// Deallocate dynamic memory created in rpthread_mutex_init
	// rpthread_mutex_t* mx;
	// *mx = *mutex;
	// free(mutex->mutexBlocked);
	return 0;
};

/* scheduler */
static void schedule() {
	// Every time when timer interrup happens, your thread library 
	// should be contexted switched from thread context to this 
	// schedule function

	// Invoke different actual scheduling algorithms
	// according to policy (RR or MLFQ)

	// YOUR CODE HERE
	inMutex = 1;
	// schedule policy
	#ifndef MLFQ
		// Choose RR
		sched_rr();
	#else 
		// Choose MLFQ
		sched_mlfq();
	#endif

}

/* Round Robin (RR) scheduling algorithm */
static void sched_rr() {
    // Your own implementation of RR
    // (feel free to modify arguments and return types)
    // YOUR CODE HERE
    int status = head[currentLevel]->tcb->status;

    if(status==KILL){ //coming from exit
        Node* temp=head[currentLevel];
        head[currentLevel] =head[currentLevel]->next;
        free(temp->tcb->context->uc_stack.ss_sp);
        free(temp->tcb->context);
        free(temp->tcb);
        free(temp);
    }
    else if(status==SCHEDULED || status == TIME){ //coming from yield
        if(head[currentLevel]->next!=NULL){
            Node* temp = head[currentLevel];
            head[currentLevel] = head[currentLevel]->next;
            temp->next = NULL;
            tail[currentLevel]->next = temp;
            tail[currentLevel] = tail[currentLevel]->next;
        }
    }
    else if(status == BLOCKED){
        Node* temp=head[currentLevel];
        head[currentLevel]=head[currentLevel]->next;
		temp->next = NULL;
        enqueueBlocked(temp);
    }
	nextThread();
   
}

/* Preemptive MLFQ scheduling algorithm */
static void sched_mlfq() {
	// Your own implementation of MLFQ
	// (feel free to modify arguments and return types)

	// YOUR CODE HERE
	int status = head[currentLevel]->tcb->status;
	if(status == TIME){
		Node* temp = head[currentLevel];
		head[currentLevel] = head[currentLevel]->next;
		temp->next = NULL;
		if(currentLevel == 3)
			enqueue(temp, currentLevel);
		else 
			enqueue(temp, currentLevel+1);
		
	}else sched_rr();


	nextThread();
}

// Feel free to add any other functions you need

// YOUR CODE HERE
void enqueue(Node* node, int level){
	node->tcb->priority=level;
    if(head[level] == NULL){
        head[level] = node;
        tail[level] = head[level];
        return;
    }
    tail[level]->next = node;
	tail[level] = tail[level]->next;
	tail[level]->next = NULL;
}

void nextThread(){
	for(int i=0; i<levels; i++){
		if(head[i]!= NULL){
			head[i]->tcb->status=READY;
			resetTimer();
			currentLevel = i;
			inMutex = 0;
			setcontext(head[i]->tcb->context);
		}
	}
	printf("End of program?\n");
	printList();
	printf("Mutex List");
	exit(0);
}

void cleanup (void) {
	if(isInit!=0){
		free(schedContext->uc_stack.ss_sp);
		free(schedContext);
		free(exitContext->uc_stack.ss_sp);
		free(exitContext);

		free(mainNode->tcb->context);
		free(mainNode->tcb);
		free(mainNode);

		free(head);
		free(tail);
	}
}