/*
 * Scheduler.
 *
 * The default scheduler is very simple, just a round-robin run queue.
 * You'll want to improve it.
 */

#include <types.h>
#include <lib.h>
#include <scheduler.h>
#include <thread.h>
#include <machine/spl.h>
#include <queue.h>

/*
 *  Scheduler data
 */

// Queue of runnable threads
static struct queue *runqueue;

/*
 * Setup function
 */
void
scheduler_bootstrap(void)
{
	int i;
	//initialize NUM_PRIORITIES queues, one for each priority level
	runqueue[NUM_PRIORITIES];
	for(i = 0; i < NUM_PRIORITIES; i++)
	{
		runqueue[i] = q_create(32);
	}
	
	if (runqueue == NULL) 
	{
		panic("scheduler: Could not create run queue\n");
	}
}

/*
 * Ensure space for handling at least NTHREADS threads.
 * This is done only to ensure that make_runnable() does not fail -
 * if you change the scheduler to not require space outside the 
 * thread structure, for instance, this function can reasonably
 * do nothing.
 */
 
 // modified to include which priority queue the thread is in
int
scheduler_preallocate(int nthreads, int priority)
{
	assert(curspl>0);
	return q_preallocate(runqueue[priority], nthreads);
}

/*
 * This is called during panic shutdown to dispose of threads other
 * than the one invoking panic. We drop them on the floor instead of
 * cleaning them up properly; since we're about to go down it doesn't
 * really matter, and freeing everything might cause further panics.
 */
void
scheduler_killall(void)
{
	int i;
	assert(curspl>0);
	
	//kill all queues
	for(i = 0; i < NUM_PRIORITIES; i++)
	{
		while (!q_empty(runqueue[i])) 
		{
			struct thread *t = q_remhead(runqueue[i]);
			kprintf("scheduler: Dropping thread %s.\n", t->t_name);
		}
	}
}

/*
 * Cleanup function.
 *
 * The queue objects to being destroyed if it's got stuff in it.
 * Use scheduler_killall to make sure this is the case. During
 * ordinary shutdown, normally it should be.
 */
void
scheduler_shutdown(void)
{
	int i;
	scheduler_killall();

	assert(curspl>0);
	
	// cleanup all queues
	for(i = 0; i < NUM_PRIORITIES; i++)
	{
		q_destroy(runqueue[i]);
		runqueue[i] = NULL;
	}
}

/*
 * Actual scheduler. Returns the next thread to run.  Calls cpu_idle()
 * if there's nothing ready. (Note: cpu_idle must be called in a loop
 * until something's ready - it doesn't know whether the things that
 * wake it up are going to make a thread runnable or not.) 
 */
struct thread *
scheduler(void)
{
	int runme, i, allempty = 1;
	// meant to be called with interrupts off
	assert(curspl>0);
	
	//while all queues are empty, idle the CPU
	while ( allempty = 1)
	{
		for(i = 0; i < NUM_PRIORITIES; i++)
		{
			if(!q_empty(runqueue[i]))
				 allempty = 0;
		}
		
		cpu_idle();
	}
	
	// find the highest priority queue with stuff in it
	for(i = NUM_PRIORITIES; i >= 0 ; i--)
	{
		if(!q_empty(runqueue[i])
			runme = i;
	}

	// You can actually uncomment this to see what the scheduler's
	// doing - even this deep inside thread code, the console
	// still works. However, the amount of text printed is
	// prohibitive.
	// 
	//print_run_queue();
	
	return q_remhead(runqueue[runme]);
}

/* 
 * Make a thread runnable.
 * With the base scheduler, just add it to the end of the run queue.
 */
int
make_runnable(struct thread *t)
{
	// meant to be called with interrupts off
	assert(curspl>0);

	return q_addtail(runqueue[t->priority], t);
}

/*
 * Debugging function to dump the run queue.
 */
void
print_run_queue(void)
{
	/* Turn interrupts off so the whole list prints atomically. */
	int spl = splhigh();

	int i,j,k=0;
	for(j = 0; j < NUM_PRIORITIES)
	{
		i = q_getstart(runqueue[j]);
		
		while (i!=q_getend(runqueue[j])) 
		{
			struct thread *t = q_getguy(runqueue[j], i);
			kprintf("  %2d: %s %p\n", k, t->t_name, t->t_sleepaddr);
			i=(i+1)%q_getsize(runqueue[j]);
			k++;
		}
	}
	
	splx(spl);
}
