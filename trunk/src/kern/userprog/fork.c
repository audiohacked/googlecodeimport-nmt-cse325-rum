#include <types.h>
#include <kern/unistd.h>
#include <kern/errno.h>
#include <lib.h>
#include <addrspace.h>
#include <thread.h>
#include <curthread.h>
#include <vm.h>
#include <vfs.h>
#include <test.h>
#include <array.h>

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
	int result;

	/* check for space for child in process table */
	result = thread_fork(strcat(curthread->t_name, "_child"), NULL, 0, NULL, &child);
	if (result)
	{
		return -result;
	}
	
	/* copy parent's memory space to child thread */
	result = as_copy(curthread->t_vmspace, &child->t_vmspace);
	if (result)
	{
		return -result;
	}
	
	/* copy the priority of parent to child */
	child->priority = curthread->priority;
	
	/* allocate pid for child */
	ptable->parent_pid = curthread->t_ptable.process_id;
	ptable->process_id = pidcount+1;
	array_setguy(process_table, pidcount+1, ptable);
	child->t_ptable = *ptable;

	pidcount++;

	/* send return codes to parent and child */
	if (child == curthread)
	{
		return 0;
	}
	else
	{
		return child->t_ptable.process_id;
	}

}
