Corey Brown
Sean Nelson
Andrew Murtland

Please note that we are using kernel version 2.6.28.8 because of compatibility issues. Sean is on Mac OSX, and the lab version does not compile readily, so he found a version that does, with less configuring effort. I'm (Corey) on 64-bit Linux, which is the same story. We made sure this was ok with Song, and he said it was fine as long as we demonstrated that it worked to the TA.

1.We needed a mechanism that keeps track of how many groups are active.

2.We needed to modify scheduler() to include a conditional that used our scheduling method if activated by the program.

The function “asmlinkage void sched schedule(void)” is located at line 4440 in kernel version 2.6.28.8 – this is the main scheduler function. schedule() uses a run queue (rq), which is used to implement round robin scheduling.

---

Detailed explanation of what schedule does:

1.schedule() disables pre-emption and gets the rq from the cpu.

2.Not too sure what rcu\_qsctr\_inc does, except it's passed the cpu, is located in rcupreempt.h, and increments the sched\_qs of the rcu\_dyntick\_sched struct

3.It then sets the current process to be prev, switch\_count to prev->nivcsw (the number of context switches for that process), and calls release\_kernel\_lock(prev)

4.It then calls schedule\_debug, which, according to the comments, does “various schedule()-time debugging checks and statistics”, and is passed prev

5.schedule() uses “spin\_lock\_irq(&rq->lock);” to turn off interrupts. It then calls update\_rq\_clock(rq) @ line 641, which in turn sets rq->clock equal to sched\_clock\_cpu(cpu\_of(rq)), which is in sched\_clock.c @ line 149, and which apparently further disables interrupts using another spin lock, and calls update\_sched\_clock(struct sched\_clock\_data **scd, u64 now) @ line 109 in the same file – this is the function that actually increments the clock.**

6.Everything goes back to the main schedule() function, which uses a chain of functions, starting with clear\_tsk\_need\_resched to clear a bit in the prev thread (the thread that's about to go to the end of the line).

7.The kernel then deactivates the task (lines 4467 - 4472), puts the task prev onto the end of rq using put\_prev\_task(rq, prev), and uses pick\_next\_task(rq, prev) to get the next task.

8.It then increments rq->switches, sets the current task to next, and increments switch\_count

9.finally, it switches context, which should start running next, and, according to the comment, also “unlocks the rq” (line 4493)

10.it may slip through to a few gotos, which skip back to previous steps


---

3.We needed to modify sched.h to include our scheduler mode

Within sched.h are definitions of each scheduler mode, and a number associated with them. This is so that we can use “sched\_setscheduler(pid\_t pid)” to set the scheduler policy, rather than modifying the scheduler that is in place. As required, we added a “#define SCHED\_FSG” in addition to the pre-existing scheduler modes.