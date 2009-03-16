#include <thread.h>

void
_exit(int exitcode)
{
	int result;
	struct proc_table *ptable;
	pid_t curpid;
	
	curthread->t_ptable.exit_code = exitcode;
	curpid = curthread->t_ptable.process_id;
	ptable = array_getguy(process_table, curpid);
	assert(ptable->process_id == curpid);
	ptable->exit_code = exitcode;
	ptable->exited = 1;
	kill_curthread();
}