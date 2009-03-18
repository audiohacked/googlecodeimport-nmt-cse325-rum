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

extern int errno;

int get_args_count(char **args);

/* General steps to execv (found in the minix3 book):
 *      1. check permissions -- is the file executable?
 *      2. read the header to get the segment and total sizes
 *      3. fetch the arguments and environment from the caller
 *      4. allocate new memory and release unneeded old memory
 *      5. copy stack to new memory image
 *      6. copy data (and possibly text) segment to new memory image
 *      7. check for and handle setuid, setgid bits
 *      8. fix up process table entry
 *      9. tell kernel that process is now runnable 
 */
int
execv(const char *program, char **args)
{
	struct vnode *v;
	vaddr_t entrypoint, stackptr;
	struct addrspace *new, *old;
	int result, program_argc, i;
	char **program_args;
	size_t copystrlen;

	/* Open the file. */
	result = vfs_open((char *)program, O_RDONLY, &v);
	if (result) {
		//errno = result;
		//return -1;
		return -result;
	}

	/* Create a new address space. */
	old = curthread->t_vmspace;
	new = as_create();
	if (curthread->t_vmspace==NULL) {
		vfs_close(v);
		//errno = ENOMEM;
		//return -1;
		return -ENOMEM;
	}
	else
	{
		curthread->t_vmspace = new;
		as_destroy(old);
	}

	/* Activate it. */
	as_activate(curthread->t_vmspace);

	/* Load the executable. */
	result = load_elf(v, &entrypoint);
	if (result) {
		/* thread_exit destroys curthread->t_vmspace */
		vfs_close(v);
		//errno = result;
		//return -1;
		return -result;
	}

	/* Done with the file now. */
	vfs_close(v);

	/* Define the user stack in the address space */
	result = as_define_stack(curthread->t_vmspace, &stackptr);
	if (result) {
		/* thread_exit destroys curthread->t_vmspace */
		//errno = result;
		//return -1;
		return -result;
	}

	/* fetch the arguments and environment from the caller */
	program_argc = get_args_count( args );
	for(i=0; i<program_argc; i++)
	{
		copyinstr((userptr_t)args[i], program_args[i], sizeof(args[i]), &copystrlen);
	}
	program_args[program_argc] = NULL;
	copyin((userptr_t)args, &stackptr, sizeof(args));
	
	/* Warp to user mode. */
	md_usermode(program_argc, (userptr_t)program_args /*userspace addr of argv*/,
		    stackptr, entrypoint);
	
	/* md_usermode does not return */
	panic("md_usermode returned\n");
	//errno = EINVAL;
	//return -1;
	return -EINVAL;
}

int get_args_count(char **args)
{
	int retcount=0, i=-1;
	while(args[i++] != NULL)
		if (strlen(args[i]) > 0)
			retcount++;
	return retcount;
}
