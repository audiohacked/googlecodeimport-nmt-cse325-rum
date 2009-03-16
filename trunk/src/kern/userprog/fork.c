#include <thread.h>

extern int errno;

/*
 * General steps to fork (found in the minix3 book):
 *      check to see if process table is full
 *      try to allocate memory for the child's data and stack
 *      copy the parent's data and stack to the child's memory
 *      find a free process slot and copy parent's slot to it
 *      enter child's memory map in process table
 *      choose a pid for the child
 *      tell kernel and file system about child
 *      report child's memory map to kernel
 *      send return codes to parent and child. 
 */
pid_t
fork(void)
{
	struct thread *child;
	struct proc_table *ptable;
	int result, s;

	/* check for space for child in process table */
	child = thread_create(strcat(curthread->t_name, "_child"));
	if (child == NULL)
	{
		errno = ENOMEM;
		return -1;
	}
	
	/* copy parent's memory space to child thread */
	result = as_copy(curthread->t_vmspace, &child->t_vmspace);
	if (result)
	{
		errno = result;
		return -1;
	}
	
	/* Allocate a stack */
	child->t_stack = kmalloc(STACK_SIZE);
	if (child->t_stack==NULL) {
		kfree(child->t_name);
		kfree(child);
		return ENOMEM;
	}

	/* stick a magic number on the bottom end of the stack */
	child->t_stack[0] = 0xae;
	child->t_stack[1] = 0x11;
	child->t_stack[2] = 0xda;
	child->t_stack[3] = 0x33;

	/* Inherit the current directory */
	if (curthread->t_cwd != NULL) {
		VOP_INCREF(curthread->t_cwd);
		child->t_cwd = curthread->t_cwd;
	}

	/* Set up the pcb (this arranges for func to be called) */
	md_initpcb(&child->t_pcb, child->t_stack, NULL, 0, NULL);


	/* Interrupts off for atomicity */
	s = splhigh();

	/*
	 * Make sure our data structures have enough space, so we won't
	 * run out later at an inconvenient time.
	 */
	result = array_preallocate(sleepers, totalthreads+1);
	if (result)
	{
		goto fail;
	}

	result = array_preallocate(zombies, totalthreads+1);
	if (result)
	{
		goto fail;
	}

	result = array_preallocate(process_table, totalthreads+1);
	if (result)
	{
		goto fail;
	}

	/* Do the same for the scheduler. */
	result = scheduler_preallocate(numthreads[get_priority(child)]+1, get_priority(child));
	if (result)
	{
		goto fail;
	}

	/* copy the priority of parent to child */
	child->priority = curthread->priority;
	
	/* allocate pid for child */
	ptable->parent_pid = curthread->t_ptable.proccess_id;
	ptable->process_id = totalthreads+1;
	array_setguy(process_table, totalthreads+1, ptable);
	child->t_ptable = *ptable;

	/* tell kernel about child */
	result = make_runnable(child);
	if (result)
	{
		goto fail;
	}

	/*
	 * Increment the thread counter. This must be done atomically
	 * with the preallocate calls; otherwise the count can be
	 * temporarily too low, which would obviate its reason for
	 * existence.
	 */
	numthreads[get_priority(child)]++;
	totalthreads++;

	/* Done with stuff that needs to be atomic */
	splx(s);

	/* send return codes to parent and child */
	if (child == curthread)
	{
		return 0;
	}
	else
	{
		return child->t_ptable.process_id;
	}

 fail:
	splx(s);
	if (child->t_cwd != NULL) {
		VOP_DECREF(child->t_cwd);
	}
	kfree(child->t_stack);
	kfree(child->t_name);
	kfree(child);

	errno = EAGAIN;
	return -1
}
