<p>Note we're using 2.6.29 instead of the class version.</p>

`schedule()` location: sched.c @ line 4546

`scheduler_tick()`: sched.c @ 4387, critical section:

```
spin_lock(&rq->lock);
        update_rq_clock(rq);
        update_cpu_load(rq);
        curr->sched_class->task_tick(rq, curr, 0);
spin_unlock(&rq->lock);
```

`effective_prio(struct task_struct *p)`: sched.c @ line 1759

`sched_fork(struct task_struct *p, int clone_flags)`: sched.c @ line 2422

`__sched_fork(struct task_struct *p)`: sched.c @ line 2382

rq is the run queue