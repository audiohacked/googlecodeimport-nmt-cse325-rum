#ifndef __PID_H
#define __PID_H

struct array;

struct proc_table
{
	pid_t parent_pid;
	pid_t process_id;
	int exit_code;
	int exited;
};

extern struct array *process_table;
extern int pidcount;

#endif /* __PID_H */
