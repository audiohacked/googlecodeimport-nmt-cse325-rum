#include <types.h>
#include <kern/unistd.h>
#include <kern/errno.h>
#include <lib.h>
#include <addrspace.h>
#include <thread.h>
#include <curthread.h>
#include <array.h>
#include <pid.h>

void
_exit(int exitcode)
{
	struct proc_table *ptable;
	pid_t curpid;
	
	curthread->t_ptable.exit_code = exitcode;
	curpid = curthread->t_ptable.process_id;
	ptable = array_getguy(process_table, curpid);
	assert(ptable->process_id == curpid);
	ptable->exit_code = exitcode;
	ptable->exited = 1;
	thread_exit();
}
