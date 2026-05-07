
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include "timeout.h"

//#define MAINDEBUG(x)	printf(x)


static struct callout *callout = NULL;	/* Callout list */
static struct timeval timenow;		/* Current time */

/*
 * timeout - Schedule a timeout.
 *
 * Note that this timeout takes the number of seconds, NOT hz (as in
 * the kernel).
 */
void
timeout(func, arg, time, handle)
    void (*func) __P((void *));
    void *arg;
    int time;
    struct callout *handle;
{
    struct callout *p = callout;

    //MAINDEBUG(("Timeout %p:%p in %d seconds.", func, arg, time));
  
    handle->c_arg = arg;
    handle->c_func = func;
    gettimeofday(&timenow, NULL);
    handle->c_time.tv_sec = timenow.tv_sec + time;
    handle->c_time.tv_usec = timenow.tv_usec;

	/* try to find out the handle */
    while (p) {
	    if(p == handle) {
			return;
		}
		p = p->c_next;
    }
  
	/* put handle in the front */
    handle->c_next = callout;
    callout = handle;
}


/*
 * untimeout - Unschedule a timeout.
 */
void
untimeout(handle)
struct callout *handle;
{
	struct callout **q, *p;
  
    //MAINDEBUG(("Untimeout %p:%p.", func, arg));
  
	/* Remove the entry matching timeout and remove it from the list. */
	for (q = &callout; (p = *q); q = &p->c_next)
		if (p == handle) {
			*q = p->c_next;
			break;
		}

}


/*
 * calltimeout - Call any timeout routines which are now due.
 */
void
calltimeout()
{
    struct callout *p = callout;
	
	if (gettimeofday(&timenow, NULL) < 0)
	    //fatal("Failed to get time of day: %m");
    	printf("Failed to get time of day: %m");

    while (p) {
	    if((timenow.tv_sec > p->c_time.tv_sec) || \
	    	(timenow.tv_sec == p->c_time.tv_sec && timenow.tv_usec >= p->c_time.tv_usec)) {
			(*p->c_func)(p->c_arg);
		}
		p = p->c_next;
    }
}
