#ifndef __PID_H
#define __PID_H

#include <types.h>

struct proc_table
{
	pid_t parent_pid;
	pid_t process_id;
}

#endif /* __PID_H */