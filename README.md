# get_number_of_enter_wait_queue

## problem
寫一個system call，讓process可以是用他得到進入waiting queue的次數。

## 程式做法
在include/linux中的task_struct新增一個變數enter_queue_counter，用來計算總共進入waiting queue幾次。
:::info
如果將變數加到task_struct中的任意位置，會產生問題。因為context switch是靠固定的位移記憶體大小來得到task_struct其中的變數，交換process的
:::

```c=
...
#ifdef CONFIG_SECURITY
        /* Used by LSM modules for access restriction: */
        void                            *security;
#endif
        unsigned int enter_queue_counter; //here

        /*
         * New fields for task_struct should be added above here, so that
         * they are included in the randomized portion of task_struct.
         */
        randomized_struct_fields_end
...
```

在/kernel/fork.c的copy_mm中進行enter_queue_counter的初始化
```c=
static int copy_mm(unsigned long clone_flags, struct task_struct *tsk)
{
        struct mm_struct *mm, *oldmm;
        int retval;

        tsk->enter_queue_counter = 0; //here
        tsk->min_flt = tsk->maj_flt = 0;
        tsk->nvcsw = tsk->nivcsw = 0;
#ifdef CONFIG_DETECT_HUNG_TASK
        tsk->last_switch_count = tsk->nvcsw + tsk->nivcsw;
#endif
...

```
欲計算進入waiting queue的次數，等同於計算有多少個process(task)執行wake up。
trace 路徑:  [default_wake_function](https://elixir.free-electrons.com/linux/v3.9/source/kernel/sched/core.c#L3109)->try_to_wake_up->ttwu_queue->ttwu_do_activate->[ttwu_do_wakeup](https://elixir.free-electrons.com/linux/v3.9/source/kernel/sched/core.c#L1289)。
每次執行wake up時，counter+1。
```c=
/*
 * Mark the task runnable and perform wakeup-preemption.
 */
static void ttwu_do_wakeup(struct rq *rq, struct task_struct *p, int wake_flags,
                           struct rq_flags *rf)
{
        check_preempt_curr(rq, p, wake_flags);
        p->enter_queue_counter++; //here
        p->state = TASK_RUNNING;
        trace_sched_wakeup(p);

#ifdef CONFIG_SMP
...
```
### 輸出結果
![](https://i.imgur.com/YSBl5LW.png)

![](https://i.imgur.com/pYPUWdx.png)
