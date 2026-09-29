# Probable New xv6 Scheduling Lab Questions

## Overview

Imagine you are a teacher preparing a new lab test on **CPU scheduling in xv6**.

The previous lab questions already covered:

- **A1:** Preemptive Shortest Job First (SJF)
- **A2:** Priority-Based Scheduling
- **B1:** First Come First Served (FCFS)
- **B2:** Priority-Based Scheduling with Aging
- **C2:** Longest Weighted Wait (LWW)

The following questions are intentionally different from those algorithms. Each problem requires modifying the xv6 scheduler and, where necessary, adding process fields and system calls.

> **Important:** These are new probable questions and do not repeat SRTF, dynamic Lottery, Stride, MLFQ, CFS/vruntime, SJF, FCFS, simple Priority, Priority + Aging, or LWW.

---

# Problem 1: Round Robin with Configurable Time Quantum

## Question Statement

Modify the xv6 scheduler to implement **Round Robin scheduling with a user-configurable time quantum**.

The default time quantum is **5 timer ticks**. A process may change its own quantum using a new system call:

```c
int setquantum(int q);
```

The scheduler should:

1. Give every RUNNABLE process a CPU turn.
2. Run the selected process for at most its configured quantum.
3. Force the process to yield when its quantum expires.
4. Continue from the next RUNNABLE process.
5. If a process finishes, blocks, or voluntarily yields before the quantum expires, immediately continue scheduling normally.
6. A quantum smaller than 1 must be rejected.

Each process starts with a default quantum of 5 ticks.

---

## Given Code (`testloop.c`)

```c
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  int pid = getpid();
  int quantum = atoi(argv[1]);
  int iters = atoi(argv[2]);

  if(setquantum(quantum) < 0){
    printf("Invalid quantum\n");
    exit(1);
  }

  printf("PID %d: quantum=%d, start=%d\n",
         pid, getquantum(), uptime());

  for(int i = 0; i < iters; i++){
    for(volatile int j = 0; j < 20000000; j++)
      ;
    printf("PID %d: iteration %d at %d\n",
           pid, i + 1, uptime());
  }

  printf("PID %d: finished at %d\n", pid, uptime());
  exit(0);
}
```

---

## Required Changes

### Step 1: Add quantum fields to `proc.h`

Add:

```c
int quantum;
int ticks_used;
```

`quantum` stores the process's configured time slice.

`ticks_used` stores how many timer ticks the process has consumed during its current turn.

### Step 2: Add system calls

In `kernel/syscall.h`:

```c
#define SYS_setquantum 27
#define SYS_getquantum 28
```

In `sysproc.c`:

```c
uint64
sys_setquantum(void)
{
  int q;

  if(argint(0, &q) < 0)
    return -1;

  if(q < 1)
    return -1;

  myproc()->quantum = q;
  return 0;
}

uint64
sys_getquantum(void)
{
  return myproc()->quantum;
}
```

### Step 3: Initialize the process

In `allocproc()`:

```c
p->quantum = 5;
p->ticks_used = 0;
```

### Step 4: Reset the quantum when a new turn begins

Before a process is dispatched:

```c
p->ticks_used = 0;
```

### Step 5: Modify the timer/yield path

The timer interrupt should increase:

```c
myproc()->ticks_used++;
```

When:

```c
myproc()->ticks_used >= myproc()->quantum
```

the process should yield.

### Step 6: Add user declarations

In `user/user.h`:

```c
int setquantum(int);
int getquantum(void);
```

In `user/usys.S`:

```asm
entry("setquantum");
entry("getquantum");
```

---

## Algorithm Explanation

Round Robin maintains fairness by giving each process a limited CPU time slice.

Example:

```text
P1 quantum = 3
P2 quantum = 3
P3 quantum = 3

P1 -> P2 -> P3 -> P1 -> P2 -> P3
```

A process that uses its complete quantum is moved to the back of the scheduling rotation.

---

# Problem 2: Highest Response Ratio Next (HRRN) Scheduling

## Question Statement

Implement **Highest Response Ratio Next (HRRN)** scheduling in xv6.

For every RUNNABLE process calculate:

```text
Response Ratio = (Waiting Time + Estimated Service Time)
                 / Estimated Service Time
```

or equivalently:

```text
Response Ratio = 1 + Waiting Time / Service Time
```

The scheduler selects the RUNNABLE process with the highest response ratio.

The scheduling policy is **non-preemptive**: once a process is selected, it runs until it yields, blocks, or exits.

Each process receives an estimated service time when it is created.

Default service time:

```text
10 ticks
```

The user program can change its estimate with:

```c
int setservice(int);
```

---

## Given Code (`testloop.c`)

```c
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  int service = atoi(argv[1]);
  int iters = atoi(argv[2]);

  setservice(service);

  printf("PID %d: service=%d start=%d\n",
         getpid(), getservice(), uptime());

  for(int i = 0; i < iters; i++){
    for(volatile int j = 0; j < 20000000; j++)
      ;
  }

  printf("PID %d: finish=%d\n", getpid(), uptime());
  exit(0);
}
```

---

## Required Process Fields

Add to `struct proc`:

```c
uint arrival_time;
int service_time;
```

`arrival_time` records when the process became RUNNABLE.

`service_time` stores its estimated CPU requirement.

---

## System Calls

Add:

```c
#define SYS_setservice 29
#define SYS_getservice 30
```

Implementation:

```c
uint64
sys_setservice(void)
{
  int service;

  if(argint(0, &service) < 0)
    return -1;

  if(service <= 0)
    return -1;

  myproc()->service_time = service;
  return 0;
}

uint64
sys_getservice(void)
{
  return myproc()->service_time;
}
```

---

## Initialization

In `allocproc()`:

```c
p->arrival_time = 0;
p->service_time = 10;
```

When the process becomes RUNNABLE:

```c
acquire(&tickslock);
p->arrival_time = ticks;
release(&tickslock);
```

---

## Scheduler Logic

At every scheduling decision:

```text
waiting_time = current_ticks - arrival_time

ratio = 1 + waiting_time / service_time
```

Because xv6 kernel code can avoid floating-point arithmetic, compare two processes using cross multiplication:

```text
ratioA > ratioB

becomes

(serviceA + waitA) * serviceB
>
(serviceB + waitB) * serviceA
```

This avoids floating-point calculations.

---

## Example

Suppose:

```text
P1: waiting = 10, service = 5
P2: waiting = 12, service = 10
P3: waiting = 4,  service = 2
```

Then:

```text
P1 = 1 + 10/5 = 3
P2 = 1 + 12/10 = 2.2
P3 = 1 + 4/2 = 3
```

P1 and P3 tie.

Break ties using:

```text
earlier arrival_time
```

---

## Important Requirement

The scheduler must be **non-preemptive**.

Once selected:

```c
p->state = RUNNING;
```

and the process continues until its next natural scheduling point.

---

# Problem 3: Earliest Deadline First (EDF) Scheduling

## Question Statement

Implement **Earliest Deadline First (EDF)** scheduling in xv6.

Every process has a deadline expressed as an absolute timer tick.

The scheduler always selects the RUNNABLE process with the smallest absolute deadline.

Add:

```c
int setdeadline(int ticks_from_now);
int getdeadline(void);
```

The deadline supplied by the user is relative to the current time.

For example:

```text
setdeadline(50)
```

means:

```text
absolute deadline = current_ticks + 50
```

The scheduler should prefer the process whose deadline occurs first.

If two processes have the same deadline, break the tie using the smaller PID.

---

## Given Code (`deadline_test.c`)

```c
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  int deadline = atoi(argv[1]);
  int work = atoi(argv[2]);

  if(setdeadline(deadline) < 0){
    printf("Invalid deadline\n");
    exit(1);
  }

  printf("PID %d: deadline=%d start=%d\n",
         getpid(), getdeadline(), uptime());

  for(int i = 0; i < work; i++){
    for(volatile int j = 0; j < 20000000; j++)
      ;
  }

  printf("PID %d: finished at %d\n", getpid(), uptime());
  exit(0);
}
```

---

## Process Field

Add:

```c
uint deadline;
```

---

## System Calls

Add:

```c
#define SYS_setdeadline 31
#define SYS_getdeadline 32
```

Implementation:

```c
uint64
sys_setdeadline(void)
{
  int relative;

  if(argint(0, &relative) < 0)
    return -1;

  if(relative <= 0)
    return -1;

  acquire(&tickslock);
  myproc()->deadline = ticks + relative;
  release(&tickslock);

  return 0;
}

uint64
sys_getdeadline(void)
{
  return myproc()->deadline;
}
```

---

## Default Deadline

If a process does not explicitly call `setdeadline()`, assign:

```text
current_ticks + 100
```

when it becomes RUNNABLE.

---

## Scheduler Rule

Find the RUNNABLE process with:

```text
minimum deadline
```

Tie-breaking:

```text
smaller PID wins
```

Example:

```text
PID   Deadline
----------------
5       80
6       50
7       65
8       50
```

The scheduler selects:

```text
PID 6
```

because PID 6 and PID 8 have the same deadline, but PID 6 is smaller.

---

## Deadline Miss Detection

Before selecting a process, print a warning for any RUNNABLE process whose deadline has passed:

```c
if(p->deadline <= now)
  printf("WARNING: PID %d missed deadline\n", p->pid);
```

The process should still remain eligible for scheduling.

---

## Algorithm Explanation

EDF is a deadline-oriented scheduler:

```text
Smallest deadline
       ↓
     RUN
       ↓
Next smallest deadline
```

Unlike priority scheduling, the value is based on a time deadline rather than an arbitrary priority number.

---

# Problem 4: Weighted Fair Round Robin Scheduling

## Question Statement

Implement **Weighted Fair Round Robin (WFRR)** scheduling in xv6.

Every process receives a configurable weight.

The scheduler should give processes CPU time proportional to their weights.

For example:

```text
P1 weight = 1
P2 weight = 2
P3 weight = 4
```

Over a sufficiently long period, the CPU share should approximately be:

```text
P1 : P2 : P3 = 1 : 2 : 4
```

The scheduler uses a base time quantum of **2 ticks**.

A process with weight `w` receives:

```text
effective quantum = 2 * w
```

ticks during its turn.

Add:

```c
int setweight(int);
int getweight(void);
```

---

## Given Code (`testloop.c`)

```c
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  int weight = atoi(argv[1]);
  int loops = atoi(argv[2]);

  if(setweight(weight) < 0){
    printf("Invalid weight\n");
    exit(1);
  }

  printf("PID %d: weight=%d\n", getpid(), getweight());

  for(int i = 0; i < loops; i++){
    for(volatile int j = 0; j < 20000000; j++)
      ;
  }

  printf("PID %d: finished at %d\n", getpid(), uptime());
  exit(0);
}
```

---

## Process Fields

Add:

```c
int weight;
int ticks_used;
```

---

## Weight Range

Define in `kernel/param.h`:

```c
#define MIN_WEIGHT 1
#define MAX_WEIGHT 10
#define DEFAULT_WEIGHT 1
#define BASE_QUANTUM 2
```

---

## System Calls

Add:

```c
#define SYS_setweight 33
#define SYS_getweight 34
```

Implementation:

```c
uint64
sys_setweight(void)
{
  int weight;

  if(argint(0, &weight) < 0)
    return -1;

  if(weight < MIN_WEIGHT || weight > MAX_WEIGHT)
    return -1;

  myproc()->weight = weight;
  return 0;
}

uint64
sys_getweight(void)
{
  return myproc()->weight;
}
```

---

## Scheduler Rule

For a process:

```text
quantum = BASE_QUANTUM * weight
```

Therefore:

```text
weight 1 -> 2 ticks
weight 2 -> 4 ticks
weight 3 -> 6 ticks
...
weight 10 -> 20 ticks
```

The scheduler rotates through RUNNABLE processes.

When a process consumes its effective quantum:

```text
yield()
```

is triggered.

---

## Example

Three processes:

```text
P1 weight = 1
P2 weight = 2
P3 weight = 3
```

Their effective quanta are:

```text
P1 -> 2 ticks
P2 -> 4 ticks
P3 -> 6 ticks
```

One complete round gives:

```text
P1: 2
P2: 4
P3: 6
```

Total:

```text
12 ticks
```

Approximate CPU shares:

```text
P1 = 2/12 = 16.7%
P2 = 4/12 = 33.3%
P3 = 6/12 = 50%
```

---

## Important Requirement

Do not select the process with the largest weight every time.

The scheduler must continue rotating among RUNNABLE processes and use weight only to determine how long each process receives the CPU.

---

# Problem 5: Deadline-Aware Least Slack Time First (LSTF) Scheduling

## Question Statement

Implement **Least Slack Time First (LSTF)** scheduling in xv6.

For every RUNNABLE process calculate:

```text
Slack = Deadline - Current Time - Remaining CPU Time
```

The scheduler selects the process with the **smallest slack**.

A smaller slack means the process has less time available before its deadline and should receive CPU service earlier.

Each process has:

```text
deadline
remaining_time
```

The process may configure these using:

```c
int setjob(int runtime, int deadline);
```

where `deadline` is relative to the current time.

---

## Given Code (`jobtest.c`)

```c
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  int runtime = atoi(argv[1]);
  int deadline = atoi(argv[2]);

  if(setjob(runtime, deadline) < 0){
    printf("Invalid job parameters\n");
    exit(1);
  }

  printf("PID %d: runtime=%d deadline=%d\n",
         getpid(), runtime, getdeadline());

  for(int i = 0; i < runtime; i++){
    for(volatile int j = 0; j < 20000000; j++)
      ;
  }

  printf("PID %d: completed at %d\n", getpid(), uptime());
  exit(0);
}
```

---

## Process Fields

Add:

```c
uint deadline;
int remaining_time;
int original_runtime;
```

---

## System Call

Add:

```c
#define SYS_setjob 35
```

Implementation:

```c
uint64
sys_setjob(void)
{
  int runtime;
  int relative_deadline;

  if(argint(0, &runtime) < 0)
    return -1;

  if(argint(1, &relative_deadline) < 0)
    return -1;

  if(runtime <= 0 || relative_deadline <= 0)
    return -1;

  struct proc *p = myproc();

  acquire(&tickslock);
  p->deadline = ticks + relative_deadline;
  release(&tickslock);

  p->remaining_time = runtime;
  p->original_runtime = runtime;

  return 0;
}
```

---

## Scheduler Calculation

Before selecting a process:

```text
slack =
deadline - current_ticks - remaining_time
```

The scheduler selects the RUNNABLE process with the smallest slack.

Example:

```text
Current time = 100

P1:
deadline = 150
remaining = 20
slack = 150 - 100 - 20 = 30

P2:
deadline = 140
remaining = 15
slack = 140 - 100 - 15 = 25

P3:
deadline = 170
remaining = 50
slack = 170 - 100 - 50 = 20
```

Therefore:

```text
P3 is selected
```

because:

```text
20 < 25 < 30
```

---

## Tie-Breaking

If two processes have identical slack:

```text
smaller deadline wins
```

If both slack and deadline are equal:

```text
smaller PID wins
```

---

## Updating Remaining Time

Whenever the process consumes one timer tick:

```c
if(myproc()->remaining_time > 0)
  myproc()->remaining_time--;
```

The process should be considered complete when:

```text
remaining_time == 0
```

---

## Deadline Miss

If:

```text
current_ticks > deadline
```

print:

```text
WARNING: PID %d missed its deadline
```

but continue scheduling the process normally.

---

# Common Implementation Notes

## 1. Process Locks

During scheduler scans, hold only one process lock at a time.

Use the pattern:

```c
for(p = proc; p < &proc[NPROC]; p++){
  acquire(&p->lock);

  if(p->state == RUNNABLE){
    // inspect process
  }

  release(&p->lock);
}
```

After finding the winner, reacquire only the winner's lock and verify:

```c
if(p->state == RUNNABLE)
```

before dispatching it.

---

## 2. Timer Ticks

Scheduling policies that depend on elapsed time should use the real xv6 `ticks` counter.

Read it under:

```c
acquire(&tickslock);
now = ticks;
release(&tickslock);
```

Do not assume that one pass through the scheduler loop equals one timer tick.

---

## 3. Single CPU

For deterministic testing, use:

```makefile
CPUS := 1
```

---

## 4. User System Call Files

For every new system call, remember to update the appropriate xv6 files:

```text
kernel/syscall.h
kernel/syscall.c
kernel/sysproc.c
user/user.h
user/usys.S
```

Depending on the xv6 version, `user/usys.pl` may be used instead of directly editing `user/usys.S`.

---

## 5. Testing

Create multiple processes with different scheduling parameters.

For example:

```text
$ jobtest 10 50
$ jobtest 5 20
$ jobtest 15 70
```

Run tests with enough workload so scheduling differences are visible.

Print:

```text
PID
arrival time
start time
finish time
scheduling parameter
```

to make the scheduling order easy to verify.

---

# Quick Comparison of the New Problems

| Problem | Scheduling Algorithm | Main Selection Rule | Preemptive? | Main New Data |
|---|---|---|---|---|
| 1 | Configurable Round Robin | Next process + quantum | Yes | quantum, ticks_used |
| 2 | HRRN | Highest response ratio | No | arrival_time, service_time |
| 3 | EDF | Earliest deadline | Depends on implementation | deadline |
| 4 | Weighted Fair Round Robin | Rotating turns + weighted quantum | Yes | weight, ticks_used |
| 5 | LSTF | Lowest slack | Yes / periodic | deadline, remaining_time |

---

# Why These Are Good Lab-Test Questions

These problems test different scheduling concepts:

1. **Round Robin with configurable quantum** tests timer-based preemption and context switching.
2. **HRRN** tests scheduling formulas and starvation avoidance without simply using priority.
3. **EDF** tests deadline-based real-time scheduling.
4. **Weighted Fair Round Robin** tests proportional CPU allocation.
5. **LSTF** tests combining deadlines with remaining execution time.

They also require students to understand the interaction between:

```text
proc structure
      ↓
system calls
      ↓
timer interrupts
      ↓
scheduler()
      ↓
context switching
```

The questions therefore remain close to the style of an xv6 scheduling lab while requiring algorithms different from the previously covered problems.
