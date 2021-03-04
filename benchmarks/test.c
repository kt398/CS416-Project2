#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include "../rpthread.h"


void simplef(){
	puts("Donald- you are threaded\n");
	rpthread_yield();
	puts("We're back\n");
}
/* A scratch program template on which to call and
 * test rpthread library functions as you implement
 * them.
 *
 * You can modify and use this program as much as possible.
 * This will not be graded.
 */
int main(int argc, char **argv) {

	/* Implement HERE */
	rpthread_t thread;
	rpthread_create(thread,NULL,simplef,NULL);
	
	return 0;
}
