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

pid_t
waitpid(pid_t wpid, int *status, int options)
{
	struct proc_table *ptable;

	if (status == NULL)
	{
		errno = EFAULT;
		return -1;
	}
	
	if (array_getnum(process_table) < wpid)
	{
		errno = EFAULT;
		return -1;
	}

	ptable = array_getguy(process_table, wpid);

	switch (options)
	{	
	case 0:break;
	default:
		errno = EINVAL;
		return -1;
	}

	while(!ptable->exited);
	*status = ptable->exit_code;
	return ptable->process_id;
}
