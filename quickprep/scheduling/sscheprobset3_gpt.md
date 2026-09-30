# xv6 Scheduler Problem Set 3 — Missing Algorithms

## Purpose

This problem set covers scheduling algorithms that were **not covered by the earlier xv6 set**.

Previously covered:

- A1: Preemptive SJF
- A2: Priority Scheduling
- B1: FCFS
- B2: Priority + Aging
- C2: Longest Weighted Wait (LWW)

This set focuses on the remaining important algorithms and variants:

1. Round Robin (RR)
2. Shortest Remaining Time First (SRTF)
3. Non-Preemptive Priority
4. Multilevel Queue (MLQ)
5. Multilevel Feedback Queue (MLFQ)
6. Lottery Scheduling
7. Stride Scheduling
8. Highest Response Ratio Next (HRRN)
9. Weighted Round Robin (WRR)
10. Earliest Deadline First (EDF)
11. Rate Monotonic Scheduling (RMS)
12. Fair-Share Scheduling

---

# Common xv6 Rules

Use these rules unless a problem explicitly changes them.

### 1. Single CPU

In `Makefile`:

```makefile
CPUS := 1
```

This makes scheduling traces deterministic.

### 2. Process locking

During a scheduler scan, hold only one process lock at a time:

```c
for(p = proc; p < &proc[NPROC]; p++){
  acquire(&p->lock);

  if(p->state == RUNNABLE){
    /* inspect p */
  }

  release(&p->lock);
}
```

After finding a winner, reacquire only that process's lock and re-check:

```c
acquire(&winner->lock);

if(winner->state == RUNNABLE){
  winner->state = RUNNING;
  c->proc = winner;
  swtch(&c->context, &winner->context);
  c->proc = 0;
}

release(&winner->lock);
```

### 3. Real timer ticks

For time-based algorithms, use xv6's actual `ticks` value:

```c
acquire(&tickslock);
uint now = ticks;
release(&tickslock);
```

Do not use "number of scheduler-loop iterations" as elapsed time.

### 4. System-call plumbing

For a new system call, update the appropriate files:

```text
kernel/syscall.h
kernel/syscall.c
kernel/sysproc.c
user/user.h
user/usys.pl
```

Some xv6 versions generate `user/usys.S` from `user/usys.pl`.

### 5. Test program

A useful CPU-bound test is:

```c
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  int loops = atoi(argv[1]);

  printf("PID %d start=%d\n", getpid(), uptime());

  for(int i = 0; i < loops; i++){
    for(volatile int j = 0; j < 20000000; j++)
      ;
  }

  printf("PID %d finish=%d\n", getpid(), uptime());
  exit(0);
}
```

---

# Problem 1 — Round Robin with Configurable Quantum

## Problem Statement

Implement **Round Robin (RR)** scheduling.

Every RUNNABLE process receives a fixed CPU time quantum. When its quantum expires, it must yield and the next RUNNABLE process gets the CPU.

Default quantum:

```text
5 timer ticks
```

Add:

```c
int setquantum(int q);
int getquantum(void);
```

Rules:

1. Every process starts with quantum 5.
2. `q < 1` is invalid.
3. A process runs for at most its quantum.
4. If it exits or blocks early, the scheduler continues normally.
5. The scheduler must rotate through RUNNABLE processes rather than always selecting the same one.

## Solution

### Step 1 — Add fields to `struct proc`

In `kernel/proc.h`:

```c
int quantum;
int ticks_used;
```

### Step 2 — Initialize

In `allocproc()`:

```c
p->quantum = 5;
p->ticks_used = 0;
```

### Step 3 — System calls

```c
uint64
sys_setquantum(void)
{
  int q;

  if(argint(0, &q) < 0 || q < 1)
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

Add syscall numbers and normal xv6 syscall plumbing.

### Step 4 — Scheduler

Use table-order RR:

```c
static int next_slot = 0;

for(;;){
  intr_on();

  for(int n = 0; n < NPROC; n++){
    int i = (next_slot + n) % NPROC;
    struct proc *p = &proc[i];

    acquire(&p->lock);

    if(p->state == RUNNABLE){
      p->ticks_used = 0;
      p->state = RUNNING;
      c->proc = p;

      next_slot = (i + 1) % NPROC;

      swtch(&c->context, &p->context);

      c->proc = 0;
      release(&p->lock);
      break;
    }

    release(&p->lock);
  }
}
```

### Step 5 — Timer enforcement

In the timer path, increment the running process's counter:

```c
struct proc *p = myproc();

if(p != 0){
  acquire(&p->lock);
  p->ticks_used++;

  int expired = (p->ticks_used >= p->quantum);

  if(expired)
    p->ticks_used = 0;

  release(&p->lock);

  if(expired)
    yield();
}
```

### Expected Trace

For three processes with quantum 2:

```text
P1 -> P2 -> P3 -> P1 -> P2 -> P3
```

---

# Problem 2 — Shortest Remaining Time First (SRTF)

## Problem Statement

Implement **preemptive Shortest Remaining Time First**.

Each process has:

```text
remaining_time
```

The scheduler selects the RUNNABLE process with the smallest remaining CPU requirement.

If a newly runnable process has a smaller remaining time than the currently running process, the current process must eventually be preempted by the timer path.

Default remaining time:

```text
10 ticks
```

Add:

```c
int setremaining(int);
int getremaining(void);
```

## Solution

### Step 1 — Process fields

```c
int remaining_time;
```

### Step 2 — Initialization

```c
p->remaining_time = 10;
```

A child may inherit the parent's remaining-time estimate or start with the default, depending on the assignment specification. Use one rule consistently.

### Step 3 — Syscalls

```c
uint64
sys_setremaining(void)
{
  int x;

  if(argint(0, &x) < 0 || x <= 0)
    return -1;

  myproc()->remaining_time = x;
  return 0;
}

uint64
sys_getremaining(void)
{
  return myproc()->remaining_time;
}
```

### Step 4 — Select minimum

```c
struct proc *shortest = 0;
int best = 0;
int found = 0;

for(p = proc; p < &proc[NPROC]; p++){
  acquire(&p->lock);

  if(p->state == RUNNABLE){
    if(!found || p->remaining_time < best){
      best = p->remaining_time;
      shortest = p;
      found = 1;
    }
  }

  release(&p->lock);
}
```

Then re-acquire the winner:

```c
if(shortest){
  acquire(&shortest->lock);

  if(shortest->state == RUNNABLE){
    shortest->state = RUNNING;
    c->proc = shortest;

    swtch(&c->context, &shortest->context);

    c->proc = 0;
  }

  release(&shortest->lock);
}
```

### Step 5 — Consume remaining time

On each CPU timer tick:

```c
if(p != 0){
  acquire(&p->lock);

  if(p->state == RUNNING && p->remaining_time > 0)
    p->remaining_time--;

  int should_yield =
      (p->state == RUNNING && p->remaining_time > 0);

  release(&p->lock);

  if(should_yield)
    yield();
}
```

For a real assignment, the exact preemption condition should be tied to the SRTF policy and timer path rather than blindly yielding every tick.

### Key Difference from SJF

```text
SJF  = shortest job before execution
SRTF = shortest remaining job during execution
```

---

# Problem 3 — Non-Preemptive Priority Scheduling

## Problem Statement

Implement **non-preemptive priority scheduling**.

Each process has a priority:

```text
1 = lowest
10 = highest
```

The scheduler chooses the highest-priority RUNNABLE process.

Once selected, it runs until it blocks, yields, or exits.

Unlike the previously implemented priority scheduler, this question explicitly requires the scheduling decision to be **non-preemptive**.

## Solution

### Process field

```c
int priority;
```

### Parameters

```c
#define MIN_PRIORITY 1
#define MAX_PRIORITY 10
#define DEFAULT_PRIORITY 5
```

### Selection

```c
struct proc *best = 0;
int best_priority = 0;

for(p = proc; p < &proc[NPROC]; p++){
  acquire(&p->lock);

  if(p->state == RUNNABLE &&
     (best == 0 || p->priority > best_priority)){
    best = p;
    best_priority = p->priority;
  }

  release(&p->lock);
}
```

Then run `best` using the standard two-pass lock pattern.

### Important

Do **not** add a timer-triggered priority preemption.

The timer may still perform normal xv6 timer accounting, but the scheduling policy itself does not interrupt the selected job simply because another process has a higher priority.

---

# Problem 4 — Multilevel Queue (MLQ)

## Problem Statement

Implement a **Multilevel Queue scheduler**.

Create three fixed queues:

```text
Queue 0: SYSTEM
Queue 1: INTERACTIVE
Queue 2: BATCH
```

Priority between queues is strict:

```text
SYSTEM > INTERACTIVE > BATCH
```

Each process belongs permanently to one queue.

Within each queue, use Round Robin.

Add:

```c
int setqueue(int);
int getqueue(void);
```

A process cannot move between queues.

## Solution

### Process fields

```c
int queue_id;
int ticks_used;
```

### Parameters

```c
#define SYSTEM_Q       0
#define INTERACTIVE_Q  1
#define BATCH_Q        2
#define QUEUE_QUANTUM  3
```

### Scheduler

```c
for(;;){
  intr_on();

  int scheduled = 0;

  for(int q = SYSTEM_Q; q <= BATCH_Q && !scheduled; q++){

    for(p = proc; p < &proc[NPROC]; p++){
      acquire(&p->lock);

      if(p->state == RUNNABLE && p->queue_id == q){
        p->ticks_used = 0;
        p->state = RUNNING;
        c->proc = p;

        swtch(&c->context, &p->context);

        c->proc = 0;
        release(&p->lock);

        scheduled = 1;
        break;
      }

      release(&p->lock);
    }
  }
}
```

### Important distinction

MLQ has **fixed queues**.

A process in the batch queue does not automatically move to the interactive queue.

That is the main distinction from MLFQ.

---

# Problem 5 — Multilevel Feedback Queue (MLFQ)

## Problem Statement

Implement a **Multilevel Feedback Queue**.

Use three levels:

```text
Q0: quantum = 2
Q1: quantum = 4
Q2: quantum = 8
```

New processes enter Q0.

Rules:

1. A process that consumes its complete quantum moves down one level.
2. A process that blocks before using the full quantum stays at its level.
3. Higher queues are scheduled before lower queues.
4. Every 50 timer ticks, perform a global priority boost and move all RUNNABLE processes to Q0.
5. Q0, Q1, and Q2 each use RR.

## Solution

### Process fields

```c
int mlfq_level;
int ticks_used;
uint last_boost;
```

### Parameters

```c
#define MLFQ_LEVELS 3
#define BOOST_INTERVAL 50
```

Quantum table:

```c
int quantum[MLFQ_LEVELS] = {2, 4, 8};
```

### Selection

```c
for(int level = 0; level < MLFQ_LEVELS; level++){
  for(p = proc; p < &proc[NPROC]; p++){
    acquire(&p->lock);

    if(p->state == RUNNABLE &&
       p->mlfq_level == level){

      p->ticks_used = 0;
      p->state = RUNNING;
      c->proc = p;

      swtch(&c->context, &p->context);

      c->proc = 0;
      release(&p->lock);

      goto scheduled;
    }

    release(&p->lock);
  }
}

scheduled:
;
```

### Timer logic

```c
p->ticks_used++;

if(p->ticks_used >= quantum[p->mlfq_level]){
  p->ticks_used = 0;

  if(p->mlfq_level < MLFQ_LEVELS - 1)
    p->mlfq_level++;

  release(&p->lock);
  yield();
}
```

### Priority boost

Every 50 real ticks:

```c
acquire(&tickslock);
uint now = ticks;
release(&tickslock);

if(now - last_global_boost >= BOOST_INTERVAL){
  for(p = proc; p < &proc[NPROC]; p++){
    acquire(&p->lock);

    if(p->state != UNUSED && p->state != ZOMBIE)
      p->mlfq_level = 0;

    release(&p->lock);
  }

  last_global_boost = now;
}
```

### MLQ vs MLFQ

```text
MLQ:
fixed queue membership

MLFQ:
processes move between queues based on behavior
```

---

# Problem 6 — Lottery Scheduling

## Problem Statement

Implement **Lottery Scheduling**.

Every RUNNABLE process owns a number of lottery tickets.

Example:

```text
P1 = 10 tickets
P2 = 20 tickets
P3 = 70 tickets
```

Total:

```text
100
```

P3 has approximately 70% of the scheduling selections over a long run.

Add:

```c
int settickets(int);
int gettickets(void);
```

Minimum tickets:

```text
1
```

Maximum:

```text
100
```

## Solution

### Process field

```c
int tickets;
```

### Parameters

```c
#define MIN_TICKETS 1
#define MAX_TICKETS 100
#define DEFAULT_TICKETS 10
```

### Random winner selection

First calculate total tickets:

```c
int total = 0;

for(p = proc; p < &proc[NPROC]; p++){
  acquire(&p->lock);

  if(p->state == RUNNABLE)
    total += p->tickets;

  release(&p->lock);
}
```

If:

```c
total == 0
```

there is no runnable process.

Generate:

```c
int winning_ticket = random() % total;
```

Then scan again:

```c
int cumulative = 0;

for(p = proc; p < &proc[NPROC]; p++){
  acquire(&p->lock);

  if(p->state == RUNNABLE){
    cumulative += p->tickets;

    if(cumulative > winning_ticket){
      /* p is the winner */
      p->state = RUNNING;
      c->proc = p;

      swtch(&c->context, &p->context);

      c->proc = 0;
      release(&p->lock);
      break;
    }
  }

  release(&p->lock);
}
```

Use a kernel-safe pseudo-random generator appropriate for your xv6 version.

### Important

Lottery scheduling is **probabilistic**.

A 70-ticket process does not have to run exactly 70% of the time in a short experiment.

The proportion becomes more meaningful over many selections.

---

# Problem 7 — Stride Scheduling

## Problem Statement

Implement **Stride Scheduling**.

Each process receives a number of tickets.

Calculate:

```text
stride = LARGE_NUMBER / tickets
```

Use:

```text
LARGE_NUMBER = 10000
```

Every process starts with:

```text
pass = 0
```

At each scheduling decision, select the RUNNABLE process with the smallest `pass`.

After it runs:

```text
pass += stride
```

## Solution

### Process fields

```c
int tickets;
int stride;
uint64 pass;
```

### Initialization

```c
p->tickets = 10;
p->stride = 10000 / p->tickets;
p->pass = 0;
```

### Ticket update

When tickets change:

```c
p->tickets = newtickets;
p->stride = 10000 / newtickets;

if(p->stride < 1)
  p->stride = 1;
```

### Selection

```c
struct proc *best = 0;
uint64 best_pass = 0;
int found = 0;

for(p = proc; p < &proc[NPROC]; p++){
  acquire(&p->lock);

  if(p->state == RUNNABLE){
    if(!found || p->pass < best_pass){
      best_pass = p->pass;
      best = p;
      found = 1;
    }
  }

  release(&p->lock);
}
```

### Dispatch

```c
if(best){
  acquire(&best->lock);

  if(best->state == RUNNABLE){
    best->pass += best->stride;

    best->state = RUNNING;
    c->proc = best;

    swtch(&c->context, &best->context);

    c->proc = 0;
  }

  release(&best->lock);
}
```

### Lottery vs Stride

```text
Lottery = random selection

Stride = deterministic proportional selection
```

If two processes have equal tickets, their stride values are equal.

---

# Problem 8 — Highest Response Ratio Next (HRRN)

## Problem Statement

Implement **HRRN**.

For every RUNNABLE process:

```text
Response Ratio =
    (Waiting Time + Service Time) / Service Time
```

or:

```text
Response Ratio =
    1 + Waiting Time / Service Time
```

The scheduler selects the largest ratio.

The scheduler is non-preemptive.

## Solution

### Process fields

```c
uint arrival_time;
int service_time;
```

### Default

```c
#define DEFAULT_SERVICE 10
```

### Arrival time

When a process becomes RUNNABLE:

```c
acquire(&tickslock);
p->arrival_time = ticks;
release(&tickslock);
```

### Avoid floating point

Compare:

```text
(serviceA + waitA) / serviceA
```

and:

```text
(serviceB + waitB) / serviceB
```

using:

```text
(serviceA + waitA) * serviceB
>
(serviceB + waitB) * serviceA
```

### Scheduler selection

```c
struct proc *best = 0;
int found = 0;
uint best_left = 0;
uint best_right = 1;

acquire(&tickslock);
uint now = ticks;
release(&tickslock);

for(p = proc; p < &proc[NPROC]; p++){
  acquire(&p->lock);

  if(p->state == RUNNABLE){
    uint wait = now - p->arrival_time;

    uint left =
        (p->service_time + wait);

    uint right =
        p->service_time;

    if(!found ||
       left * best_right > best_left * right){

      best = p;
      best_left = left;
      best_right = right;
      found = 1;
    }
  }

  release(&p->lock);
}
```

Then dispatch the winner normally.

### Why HRRN reduces starvation

A process that waits longer gets a larger:

```text
waiting_time / service_time
```

component.

Therefore a short job does not permanently block a long-waiting job.

---

# Problem 9 — Weighted Round Robin (WRR)

## Problem Statement

Implement **Weighted Round Robin**.

Each process has a weight:

```text
1..10
```

Base quantum:

```text
2 ticks
```

A process receives:

```text
effective quantum = BASE_QUANTUM * weight
```

Example:

```text
P1 weight 1 -> 2 ticks
P2 weight 2 -> 4 ticks
P3 weight 4 -> 8 ticks
```

The scheduler still rotates between processes.

## Solution

### Process fields

```c
int weight;
int ticks_used;
```

### Parameters

```c
#define BASE_QUANTUM 2
#define MIN_WEIGHT 1
#define MAX_WEIGHT 10
#define DEFAULT_WEIGHT 1
```

### Timer

```c
p->ticks_used++;

int quantum =
    BASE_QUANTUM * p->weight;

if(p->ticks_used >= quantum){
  p->ticks_used = 0;
  release(&p->lock);
  yield();
}
```

### Scheduler

Use normal RR table rotation.

The important part is:

```text
selection order = RR
turn length      = weight × base quantum
```

Do not replace this with "always select highest weight."

### Example

For weights:

```text
1, 2, 4
```

one round gives:

```text
P1 = 2 ticks
P2 = 4 ticks
P3 = 8 ticks
```

Total:

```text
14 ticks
```

Approximate shares:

```text
P1 = 14.3%
P2 = 28.6%
P3 = 57.1%
```

---

# Problem 10 — Earliest Deadline First (EDF)

## Problem Statement

Implement **Earliest Deadline First**.

Every process has an absolute deadline.

The scheduler selects:

```text
smallest deadline
```

Add:

```c
int setdeadline(int relative_ticks);
int getdeadline(void);
```

If:

```text
setdeadline(50)
```

is called at tick 100:

```text
deadline = 150
```

Tie-break:

```text
smaller PID
```

## Solution

### Process field

```c
uint deadline;
```

### Syscall

```c
uint64
sys_setdeadline(void)
{
  int relative;

  if(argint(0, &relative) < 0 || relative <= 0)
    return -1;

  struct proc *p = myproc();

  acquire(&tickslock);
  p->deadline = ticks + relative;
  release(&tickslock);

  return 0;
}
```

### Scheduler

```c
struct proc *best = 0;
uint best_deadline = 0;
int best_pid = 0;
int found = 0;

for(p = proc; p < &proc[NPROC]; p++){
  acquire(&p->lock);

  if(p->state == RUNNABLE){
    if(!found ||
       p->deadline < best_deadline ||
       (p->deadline == best_deadline &&
        p->pid < best_pid)){

      best = p;
      best_deadline = p->deadline;
      best_pid = p->pid;
      found = 1;
    }
  }

  release(&p->lock);
}
```

Then dispatch `best`.

### Deadline miss detection

```c
acquire(&tickslock);
uint now = ticks;
release(&tickslock);

if(p->deadline <= now)
  printf("WARNING: PID %d deadline missed\n", p->pid);
```

A missed process remains eligible.

### Important

EDF is about **deadlines**, not priority numbers.

---

# Problem 11 — Rate Monotonic Scheduling (RMS)

## Problem Statement

Implement **Rate Monotonic Scheduling** for periodic tasks.

Each process has:

```text
period
execution_time
next_release
```

A shorter period means a higher static priority.

For example:

```text
P1 period = 20
P2 period = 40
P3 period = 80
```

Priority:

```text
P1 > P2 > P3
```

Unlike ordinary priority scheduling, priority is derived from the period.

## Solution

### Process fields

```c
uint period;
uint execution_time;
uint next_release;
uint remaining_time;
```

### RMS rule

```text
smaller period = higher priority
```

Selection:

```c
struct proc *best = 0;
uint best_period = 0;
int found = 0;

for(p = proc; p < &proc[NPROC]; p++){
  acquire(&p->lock);

  if(p->state == RUNNABLE){
    if(!found || p->period < best_period){
      best = p;
      best_period = p->period;
      found = 1;
    }
  }

  release(&p->lock);
}
```

### Periodic release

On the timer path:

```c
if(now >= p->next_release){
  p->remaining_time = p->execution_time;
  p->next_release += p->period;
}
```

A complete implementation should protect process fields with the appropriate locks and define exactly when a periodic job becomes RUNNABLE.

### Example

```text
P1 period = 10
P2 period = 20
P3 period = 50
```

Static priority:

```text
P1 highest
P2 middle
P3 lowest
```

### RMS vs EDF

```text
RMS = priority determined by period
EDF = priority determined by absolute deadline
```

RMS priorities normally remain fixed.

EDF priorities change as deadlines change.

---

# Problem 12 — Fair-Share Scheduling

## Problem Statement

Implement **Fair-Share Scheduling**.

Instead of assigning CPU shares directly to individual processes, assign each process to a user/group.

Each group receives a configured CPU share.

Example:

```text
Group A = 20
Group B = 30
Group C = 50
```

The scheduler should approximately distribute CPU time:

```text
A : B : C = 20 : 30 : 50
```

Processes inside the same group should share that group's CPU allocation fairly.

## Solution

### Process fields

```c
int group_id;
int group_weight;
uint64 group_usage;
uint64 proc_usage;
```

For a simple xv6 implementation, keep group accounting in a fixed kernel array:

```c
#define NGROUPS 8

struct sched_group {
  int weight;
  uint64 usage;
};
```

### Basic selection idea

First select the group with the smallest normalized usage:

```text
normalized usage = usage / weight
```

Then select a RUNNABLE process inside that group with the smallest individual usage.

### Avoid floating point

Compare:

```text
usageA / weightA
```

with:

```text
usageB / weightB
```

using:

```text
usageA * weightB
<
usageB * weightA
```

### Group selection

Conceptually:

```c
if(!found ||
   group_usage_a * group_weight_b <
   group_usage_b * group_weight_a){

  selected_group = g;
}
```

### Process selection within the group

```c
struct proc *best = 0;
uint64 lowest_usage = 0;
int found = 0;

for(p = proc; p < &proc[NPROC]; p++){
  acquire(&p->lock);

  if(p->state == RUNNABLE &&
     p->group_id == selected_group){

    if(!found || p->proc_usage < lowest_usage){
      best = p;
      lowest_usage = p->proc_usage;
      found = 1;
    }
  }

  release(&p->lock);
}
```

After each dispatch:

```c
best->proc_usage++;
groups[best->group_id].usage++;
```

### Important concept

Fair-share scheduling has two levels:

```text
GROUP FAIRNESS
      ↓
PROCESS FAIRNESS
```

This prevents one user/group with many processes from automatically taking all CPU time.

---

# Testing Problem Set

## Test 1 — RR

Create:

```text
P1 quantum 2
P2 quantum 2
P3 quantum 2
```

Expected pattern:

```text
P1 P2 P3 P1 P2 P3 ...
```

---

## Test 2 — SRTF

Create:

```text
P1 remaining = 20
P2 remaining = 5
P3 remaining = 10
```

Expected first selection:

```text
P2
```

---

## Test 3 — Non-Preemptive Priority

Create:

```text
P1 priority = 3
P2 priority = 9
P3 priority = 5
```

Expected first selection:

```text
P2
```

Once P2 starts, a newly created priority-10 process must not interrupt it if the implementation is truly non-preemptive.

---

## Test 4 — MLQ

Create:

```text
P1 SYSTEM
P2 INTERACTIVE
P3 BATCH
```

Expected queue preference:

```text
SYSTEM
INTERACTIVE
BATCH
```

But processes inside a queue should use RR.

---

## Test 5 — MLFQ

Start all processes at Q0.

A CPU-bound process should gradually move:

```text
Q0 -> Q1 -> Q2
```

A process that frequently blocks should generally remain at a higher queue.

After the boost:

```text
Q0
Q0
Q0
```

for eligible processes.

---

## Test 6 — Lottery

Use:

```text
P1 = 10 tickets
P2 = 30 tickets
P3 = 60 tickets
```

Run many scheduling decisions.

The observed shares should approach:

```text
10% : 30% : 60%
```

Do not expect exact percentages in a small sample.

---

## Test 7 — Stride

Use:

```text
P1 = 10 tickets
P2 = 20 tickets
P3 = 40 tickets
```

Their strides are approximately:

```text
P1 = 1000
P2 = 500
P3 = 250
```

Smaller stride means the process's `pass` grows more slowly, so it is selected more often.

---

## Test 8 — HRRN

At the same current tick:

```text
P1 wait = 20, service = 10
P2 wait = 10, service = 5
P3 wait = 5,  service = 10
```

Ratios:

```text
P1 = 3
P2 = 3
P3 = 1.5
```

P1/P2 require the specified tie-break rule.

---

## Test 9 — WRR

Weights:

```text
P1 = 1
P2 = 2
P3 = 4
```

Quanta:

```text
2, 4, 8
```

Long-run CPU allocation should approach the weight ratio.

---

## Test 10 — EDF

Deadlines:

```text
P1 = 80
P2 = 50
P3 = 65
```

Expected:

```text
P2 -> P3 -> P1
```

assuming all remain RUNNABLE and the policy is non-preemptive.

---

## Test 11 — RMS

Periods:

```text
P1 = 10
P2 = 20
P3 = 50
```

Static priority:

```text
P1 > P2 > P3
```

---

## Test 12 — Fair Share

Groups:

```text
A = 20
B = 30
C = 50
```

After a sufficiently long workload:

```text
A ≈ 20%
B ≈ 30%
C ≈ 50%
```

The exact short-run result will vary.

---

# Algorithm Comparison

| Algorithm | Main Selection Rule | Preemptive | Main State |
|---|---|---:|---|
| RR | Next process in rotation | Yes | quantum |
| SRTF | Smallest remaining time | Yes | remaining time |
| Non-Preemptive Priority | Highest priority | No | priority |
| MLQ | Highest non-empty fixed queue | Usually yes | queue |
| MLFQ | Highest active feedback queue | Yes | level + quantum |
| Lottery | Random ticket winner | Usually yes | tickets |
| Stride | Smallest pass value | Usually yes | stride + pass |
| HRRN | Highest response ratio | No | arrival + service |
| WRR | RR with weighted quantum | Yes | weight + quantum |
| EDF | Earliest absolute deadline | Depends | deadline |
| RMS | Shortest period / highest fixed rate priority | Usually yes | period |
| Fair Share | Lowest normalized group usage | Usually yes | group usage |

---

# Important Differences to Memorize

## RR vs WRR

```text
RR:
everyone gets the same quantum

WRR:
quantum depends on weight
```

## MLQ vs MLFQ

```text
MLQ:
queue membership is fixed

MLFQ:
processes move between queues
```

## Lottery vs Stride

```text
Lottery:
random

Stride:
deterministic
```

## SJF vs SRTF

```text
SJF:
shortest job selected

SRTF:
shortest remaining job selected continuously
```

## EDF vs RMS

```text
EDF:
dynamic deadline-based priority

RMS:
fixed priority based on period
```

## Priority vs HRRN

```text
Priority:
explicit priority value

HRRN:
priority emerges from waiting time and service time
```

---

# Common Implementation Mistakes

### Mistake 1 — Holding two process locks

Bad:

```c
acquire(&winner->lock);

for(p = proc; p < &proc[NPROC]; p++){
  acquire(&p->lock);
}
```

Use one lock at a time.

### Mistake 2 — Using scheduler-loop count as time

Do not do:

```c
wait++;
```

because one scheduler pass is not necessarily one timer tick.

Use:

```c
ticks
```

for elapsed-time algorithms.

### Mistake 3 — Using floating point in the kernel

Avoid:

```c
float ratio;
```

Instead compare ratios with integer cross multiplication.

### Mistake 4 — Lottery winner selection without total tickets

Always calculate:

```text
total tickets
```

before generating the random ticket.

### Mistake 5 — Confusing MLQ with MLFQ

Fixed queues are MLQ.

Moving processes between queues is MLFQ.

### Mistake 6 — Treating Stride like Lottery

Stride does not need a random number.

It selects the smallest `pass`.

### Mistake 7 — Forgetting timer preemption

RR, SRTF, MLFQ, WRR and many other preemptive algorithms require the timer path to cause `yield()` at the appropriate time.

---

# Recommended Lab-Test Practice Order

Practice in this order:

```text
1. RR
2. SRTF
3. WRR
4. MLQ
5. MLFQ
6. Lottery
7. Stride
8. HRRN
9. EDF
10. RMS
11. Fair Share
12. Non-Preemptive Priority
```

The most important implementation concepts to master are:

```text
proc fields
    ↓
initialization
    ↓
system calls
    ↓
scheduler selection
    ↓
timer/preemption
    ↓
yield()
    ↓
context switch
```

For an exam, make sure you can write the **selection rule and the required `struct proc` fields from memory** before trying to memorize complete scheduler code.
