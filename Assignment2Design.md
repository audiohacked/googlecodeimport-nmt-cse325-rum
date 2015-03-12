# Introduction #
In this lab, we implemented many different things. The bulk of the lab was spent implementing various system calls related to file I/O and processes. The remainder of the lab was designing and implementing a scheduler.

# System Calls #
Note that using `errno` breaks the code and leads to linker problems, with the provided `errno.h` included and `errno`  and to alleviate the problem, our code just returns the negative of the error code. Therefore any calling code that checks for a positive result will still fail in the correct fashion.

## File I/O ##
For this part, we needed to implement a few things before we could implement the actual system calls - namely, file descriptors and a file descriptor table for each thread.

---

**File Descriptors** -
Our file descriptor design includes a few integral elements. Most importantly, it includes a pointer to a vnode struct, which represents the interface to a single file in the filesystem. It also includes integers which represent whether the file is readable or writable. Lastly, it includes a uio struct which represents the location in the file, and has a pointer to any data that is read from a file. We set an arbitrary maximum file limit using `MAX_FD`, and this is the maximum number of handles a thread can open (currently 255). The method we use to assign file descriptors is to just step through the array, incrementing the count by one each time, so that each subsequent file descriptor will be one higher than the previous one. Of course, `STDIN`, `STDOUT`, and `STDERR` are reserved for 0, 1, and 2 respectively.

---

**File Descriptor table and FIDs** -
Each thread has a file descriptor table of fixed size, determined by the constant `MAX_FD`, which is contained in file.h, and represents the maximum number of handles a thread can have. The table is implemented as an array, and the handle is the index into the array. Each index can only be used once. The reason for this is because subsequent function calls may try to access a handle after it has been closed using the `close` function. Thus, the handle cannot be reallocated, and we decided to set the file descriptor's `vnode` to `NULL` in order to indicate the file is closed.

---

**`open`/`close`** - use `vfs_open` and `vfs_close`, respectively, to manipulate the `struct vnode`.

---

**`read`/`write`** - checks for various error conditions, and uses `vop_read` and `vop_write`, respectively, to attempt to read the file using the `struct uio` for the location in the file and `struct vnode` as a pointer to the file.

---

**`lseek`** - seeks from the current location using the `SEEK_CUR` flag, to a specific location in the file using the `SEEK_SET` flag, and from the end of the file using the `SEEK_END` flag.

---

**`dup2`** - copies the contents of one file descriptor into another file descriptor. Very simple to implement. Error checking basically just checks to make sure the file descriptor is not already in use.

---

## Processes ##
For this part, we needed to implement a pid system to manage processes; waiting and exiting processes.

---

**PID System** -
The pid system is a `struct array` in thread.c where the index is the pid and the entry holds the parent's pid, thread's/process' pid, the exit code and if it has exited execution. Each thread will hold a copy of its pid table entry that holds the same info. `Getpid` uses the thread's pid entry to get the thread's pid. While, `waitpid` and `_exit` uses the global pid table to get their info.

---

# Scheduler #
For the scheduler redesign, we have decided to implement a priority queue. This involves creating separate queues for each respective priority. The thread code also must be repurposed in order to include and respect priorities. We have decided to normally create threads with a default priority level, which is the halfway point of the number of priorities. Both `NUM_PRIORITIES` and `NORMAL_PRIORITY` are defined in `scheduler.h` When the scheduler chooses which thread to run next, it checks every queue, starting with the lowest priority queue and moving to the highest priority queue. Thus, it finds the highest priority thread that is waiting to run on the system. The pros to this system compared to the old system is that in giving them priorities we can make sure that things that need to get though faster then others will. This however also leads to its con in that the lower priority things will have a much greater wait time. When discussing what scheduler to use we went though few other ideas like FCFS, LCFS, etc. however in the end round robin with priorities seemed to have the best over all stats of the scheduling algorithms and was not to difficult seeing that round robin was our starting point.