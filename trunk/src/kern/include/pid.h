#ifndef __PID_H
#define __PID_H

#include <types.h>

struct proc_table
{
	pid_t parent;
	pid_t process;
}

#endif /* __PID_H */