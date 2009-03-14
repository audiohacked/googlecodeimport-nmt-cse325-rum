#include <thread.h>

pid_t
getpid(void)
{
	return curthread->t_pid;
}

