#include <file.h>
#include <vfs.h>
#include <vnode.h>

extern int errno;

// open a file, without mode
/* open opens the file, device, or other kernel object named by the 
* pathname filename. The flags argument specifies how to open the file. 
* The optional mode argument is only meaningful in Unix 
* (or if you choose to implement Unix-style security later on) 
* and can be ignored.
*
* Returns a non-negative file handle, or -1 for error */
int 
open(cont char *path, int oflag, mode_t mode)
{
	/* set up variables for the vnode return, and function call return */
	int result;
	struct vnode *v;
	
	/* fix gcc warnings/errors due to unused variable */
	(void) mode; /* curthread->t_fd->mode = mode; */

	/* use vfs_open to open the file */
	result = vfs_open( path, oflag, &v);
	
	/* add new fd to process's fd table */
	curthread->t_fd->fd = curthread->fdcount++;
	curthread->t_fd->vfs_node = v;
	if (oflag & O_RDONLY) 
	{
		curthread->t_fd->writeable=0;
	}
	else
	{
		curthread->t_fd->writeable=1;
	}

	return curthread->t_fd->fd;
}

// close a file
/*The file handle fd is closed. The same file handle may then be 
* returned again from open, dup2, pipe, or similar calls. 
* Other file handles are not affected in any way, even if they are 
* attached to the same file.
*
* Returns 0 for success, -1 for error */
int close(int fid)
{
	file *fd;
	vnode *node;
	
	// find the descriptor in the process's file table
	fd = curthread->t_fd[fid];
	
	// if not a valid file handle, return -1 and set errno to EBADF
	if(fd == NULL)
	{
		errno = EBADF;
		return -1;
	}
	
	// get the vnode from the file descriptor
	node = fd->vfs_node;
	// close the node
	vfs_close(node);
	//curthread->fdcount--;
	*fd = NULL;
	return 0;
}

// read data from a file
/* read reads up to buflen bytes from the file specified by fd, 
* at the location in the file specified by the current seek position 
* of the file, and stores them in the space pointed to by buf. The 
* file must be open for reading. The current seek position of the 
* file is advanced by the number of bytes read. 
* Each read (or write) operation is atomic relative to other I/O to 
* the same file.
*
* Returns number of bytes read, 0 for EOF, or -1 for error */
int read(int fd, void *buf, size_t buflen)
{
	
}

// write data to a file
/* write writes up to buflen bytes to the file specified by fd, at the 
* location in the file specified by the current seek position of the file, 
* taking the data from the space pointed to by buf. The file must be open 
* for writing. The current seek position of the file is advanced by the 
* number of bytes written. Each write (or read) operation is atomic relative 
* to other I/O to the same file.
*
* Returns the number of bytes written, or a negative number for an error */
int write(int fd, const void *buf, size_t nbytes>)
{
	
}

//change current position in file
/* lseek alters the current seek position of the file handle filehandle, 
* seeking to a new position based on pos and whence. 
*
* Returns new position or -1 for error */
off_t lseek(int fd, off_t pos, int whence)
{
	
}

// clone a file handle
/* dup2 clones the file handle oldfd onto the file handle newfd. 
* If newfd names an open file, that file is closed. 
 The two handles refer to the same "open" of the file - that is, 
 * they are references to the same object and share the same seek pointer. 
 * Note that this is different from opening the same file twice.
 *
 * Returns newfd on success, -1 on error */
int dup2(int oldfd, int newfd)
{
	
}
