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
		return -EFAULT;
	}
	
	if (array_getnum(process_table) < wpid)
	{
		return -EFAULT;
	}

	ptable = array_getguy(process_table, wpid);

	switch (options)
	{	
	case 0:break;
	default:
		return -EINVAL;
	}

	while(!ptable->exited);
	*status = ptable->exit_code;
	return ptable->process_id;
}
