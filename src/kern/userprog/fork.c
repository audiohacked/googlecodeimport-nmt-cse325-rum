#include <thread.h>

extern int errno;

pid_t
fork(void)
{
	/* check to see if process table is full. */
	/* try to allocate memory for the child's data and stack */
	/* copy the parent's data and stack to the child's memory */
	/* find a free process slot and copy parent's slot to it */
	/* enter child's memory map in process table */
	/* choose a pid for the child */
	/* tell kernel and file system about child */
	/* report child's memory map to kernel */
	/* send return codes to parent and child. */
}
