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
int open(cont char *path, int oflag, mode_t mode)
{
	/* set up variables for the vnode return, and function call return */
	int result, fd;
	struct vnode *v;
	
	/* fix gcc warnings/errors due to unused variable */
	(void) mode; /* curthread->t_fd->mode = mode; */

	/* use vfs_open to open the file */
	result = vfs_open( path, oflag, &v);
	
	if(curthread->fdcount >= MAX_FD)
	{
		errno = EMFILE;
		return -1;
	}
	
	/* add new fd to process's fd table */
	fd = curthread->fdcount++;
	curthread->t_fd[fd].vfs_node = v;
	if (oflag & O_RDONLY)
	{
		curthread->t_fd[fd].writeable=0;
	}
	else
	{
		curthread->t_fd[fd].writeable=1;
	}
	curthread->t_fd[fd]->location.uio_offset = 0;
	
	return fd;
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
	struct file *fd;
	
	// find the descriptor in the process's file table
	fd = curthread->t_fd[fid];
	
	// if not a valid file handle, return -1 and set errno to EBADF
	if(fd->vfs_node == NULL)
	{
		errno = EBADF;
		return -1;
	}
	// close the node
	vfs_close(fd->vfs_node);
	fd->vfs_node = NULL;
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
	int result;
	
	// check for validity of file handle
	if(curthread->t_fd[fd]->vfs_node == NULL)
	{
		errno = EBADF;
		return -1;
	}
	
	//set flags of uio to match read operation
	curthread->t_fd[fd]->location.uio_rw = UIO_READ;
	curthread->t_fd[fd]->location.uio_resid = buflen;
	
	result = VOP_READ(curthread->t_fd[fd]->vfs_node, 
		curthread->t_fd[fd]->location);
	
	// if result is 0, we've reached EOF - else move data into buf
	if(result > 0)
	{
		// still need to implement actually copying data to buf
		// and checking buf for problems
		//buf = curthread->t_fd[fd]->location->uio_space.iov_un;
		//change offset here?
	}
	return result;
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
	int result;
	
	/***************** Error Checking ******************************/
	// if handle is invalid or file is not writable, return an error
	if( (curthread->t_fd[fd]->vfs_node == NULL) || 
		(curthread->t_fd[fd].writeable!=1) )
	{
		errno = EBADF;
		return -1;
	}
	// make sure buffer is initialized
	else if(buf == NULL)
	{
		errno = EFAULT;
		return -1;
	}
	// make sure there's space on filesystem (not yet sure how to do this)
	else if(space < nbytes)
	{
		errno = ENOSPC;
		return -1;
	}
	/***************** End Error Checking ***************************/
	
	// make uio writable
	curthread->t_fd[fd]->location.uio_rw = UIO_WRITE;
	// put buffer pointer into uio
	curthread->t_fd[fd]->location->uio_space.iov_un = buf;
	// resid is the number of bytes to be written
	curthread->t_fd[fd]->location.uio_resid = nbytes;
	
	result = VOP_WRITE(curthread->t_fd[fd]->vfs_node, 
	curthread->t_fd[fd]->location);
	return result;
}

//change current position in file
/* lseek alters the current seek position of the file handle filehandle, 
* seeking to a new position based on pos and whence. 
* Returns new position or -1 for error */
off_t lseek(int fd, off_t pos, int whence)
{
	// check to make sure we're not seeking past the beginning of the file
	if(whence == SEEK_CUR && 
		pos < 0 && pos > curthread->t_fd[fd]->location.uio_offset)
	{
		errno = EINVAL;
		return -1;
	}
	// check for invalid file handle
	else if(curthread->t_fd[fd]->vfs_node == NULL)
	{
		errno = EBADF;
		return -1;
	}
	else if(whence == SEEK_SET)
	{
		//probably need to check for EOF here
		//looks like vop_tryseek will help a lot
		curthread->t_fd[fd]->location.uio_offset = pos;
		return curthread->t_fd[fd]->location.uio_offset;
	}
	else if(whence == SEEK_END)
	{
		//do a read to the EOF, then add
		//pos to offset
		//need to figure out if vop_read changes the offset,
		//or I need to change it manually (same goes for read)
		return curthread->t_fd[fd]->location.uio_offset;
	}
	else
	{
		errno = EINVAL;
		return -1;
	}
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
	struct file *nfd, *ofd;
	
	// find the descriptor in the process's file table
	nfd = curthread->t_fd[newfd];
	ofd = curthread->t_fd[oldfd];
	
	// check to make sure oldfd refers to a valid handle
	if(fd->vfs_node == NULL)
	{
		errno = EBADF;
		return -1;
	}
	
	// check to see if newfd is already an open file handle
	if(fd->vfs_node != NULL)
	{
		close(newfd);
	}
	
	// increment the open count on the vfs_node
	VOP_INCOPEN(fd->vfs_node);
	
	// set all stuff in the new node equal to stuff in the old node
	nfd->vfs_node = ofd->vfs_node;
	nfd->writable = ofd->writable;
	nfd->readable = ofd->readable;
	nfd->location.uio_offset = ofd->location.uio_offset;
	return 0;
}
