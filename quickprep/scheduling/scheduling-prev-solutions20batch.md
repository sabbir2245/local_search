# xv6 Scheduler Implementation Guide

## Overview
This guide provides solutions for implementing various scheduling algorithms in xv6 kernel. Each section covers a different scheduling strategy with detailed implementation steps.

**Covered Algorithms:**
- **A1**: Preemptive Shortest Job First (SJF)
- **A2**: Priority-Based Scheduling
- **B1**: First Come First Served (FCFS)
- **B2**: Priority-Based Scheduling with Aging
- **C2**: Longest Weighted Wait (LWW) with Priority Accumulation

> **Revision note:** this version fixes concurrency and logic bugs found
> in the first draft — see "Bugs Fixed in This Revision" near the end
> for a full list. The scheduler snippets below already contain the
> corrected code.

---

## Question A1: Preemptive SJF (Shortest Job First) Scheduling

### Question Statement
Implement the preemptive SJF scheduling algorithm in xv6. The default length for each job is 10. The job length for the testloop program is its iteration count. Whenever a shorter job arrives, the scheduler will preempt the current job and run the shorter job till completion.

### Given Code (testloop.c)
```c
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char* argv[]){
    int pid = getpid();
    int entry_time = uptime();
    uint32 iters = atoi(argv[1]);
    setlength(iters);  // System call to set job length
    printf("Process %d: Starting %u iterations at time %d\n", pid, iters, entry_time);
    for(int i = 0; i < iters; i++){
        // do some dummy work
        for(int j = 0; j < 50000000; j++){
            int x = j * j;
            x = x + 1;
        }
    }
    int exit_time = uptime();
    printf("Process %d: Finished at time %d\n", pid, exit_time);
    exit(0);
}
```

### Solution

#### Step 1: Modify proc.h - Add job length tracking field
In `kernel/proc.h`, add a field to the `struct proc`:
```c
struct proc {
  // ... existing fields ...
  int job_length;  // Job length for SJF scheduling
  // ... existing fields ...
};
```

#### Step 2: Add setlength system call to syscall.h
In `kernel/syscall.h`, add:
```c
#define SYS_setlength 22  // (or next available number)
```

#### Step 3: Implement setlength in syscall.c
In `kernel/syscall.c`, add the function declaration and syscall handler:
```c
extern int sys_setlength(void);

static int (*syscalls[])(void) = {
    // ... existing syscalls ...
    [SYS_setlength] sys_setlength,
};
```

#### Step 4: Implement sys_setlength in sysproc.c
In `kernel/sysproc.c`, add:
```c
int
sys_setlength(void)
{
  int length;
  if(argint(0, &length) < 0)
    return -1;
  myproc()->job_length = length;
  return 0;
}
```

#### Step 5: Initialize job length in proc.c
In `kernel/proc.c`, in the `allocproc()` function, add:
```c
// In allocproc(), after other initializations
p->job_length = 10;  // Default job length
```

#### Step 6: Modify scheduler() in proc.c
In `kernel/proc.c`, replace the scheduler function to implement SJF:
```c
void
scheduler(void)
{
  struct proc *p;
  struct cpu *c = mycpu();
  c->proc = 0;

  for(;;){
    intr_on();

    struct proc *shortest = 0;
    int min_length = 0;
    int found = 0;

    // Pass 1: scan for the shortest-job runnable process.
    // Only ONE process lock is ever held at a time here — the old
    // version kept the current winner's lock held while continuing
    // to acquire further locks in the same scan, which can hold two
    // proc locks simultaneously and risks a lock-order deadlock.
    // A "found" flag replaces INT_MAX (undefined in the xv6 kernel —
    // there's no <limits.h> — so the original wouldn't even compile).
    for(p = proc; p < &proc[NPROC]; p++){
      acquire(&p->lock);
      if(p->state == RUNNABLE && (!found || p->job_length < min_length)){
        min_length = p->job_length;
        shortest = p;
        found = 1;
      }
      release(&p->lock);
    }

    // Pass 2: re-acquire only the winner's lock and re-check its state
    // — it may have changed between the scan and now (e.g. an
    // interrupt woke a higher-priority path or the process exited).
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
  }
}
```

#### Step 7: Add system call in user/usys.S
Add the user-side system call entry:
```asm
entry("setlength");
```

---

## Question A2: Priority-Based Scheduling

### Question Statement
Implement a simple priority-based scheduler where processes with higher priority run till completion unless a higher priority process arrives. Default priority is 300. Processes with the same priority run in Round Robin fashion.

### Given Code (testloop.c)
```c
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define BOLD_YELLOW "\033[1;33m"
#define BOLD_RED "\033[1;31m"
#define RESET "\033[0m"

int main(int argc, char* argv[]){
    int pid = getpid();
    uint32 iters = atoi(argv[1]);
    int priority = atoi(argv[2]);
    setpriority(priority);  // Set priority
    sleep(5);  // Let scheduler run
    int entry_time = uptime();
    printf(BOLD_YELLOW"PID %d (%d): Starting %u iterations at time %d\n"RESET, pid, priority, iters, entry_time);
    for(int i = 0; i < iters; i++){
        if(i%10 == 0) 
            printf("PID %d (%d): Iteration %d\n", pid, priority, i);
        for(int j = 0; j < 50000000; j++){
            int x = j * j;
            x = x + 1;
        }
    }
    int exit_time = uptime();
    printf(BOLD_RED"PID %d (%d): Finished at time %d\n"RESET, pid, priority, exit_time);
    exit(0);
}
```

### Solution

#### Step 1: Modify proc.h - Add priority fields
In `kernel/proc.h`, add to `struct proc`:
```c
struct proc {
  // ... existing fields ...
  int priority;      // Process priority
  // ... existing fields ...
};
```

#### Step 2: Add setpriority and getpriority system calls
In `kernel/syscall.h`:
```c
#define SYS_setpriority 23
#define SYS_getpriority 24
```

#### Step 3: Implement system calls in syscall.c
In `kernel/syscall.c`:
```c
extern int sys_setpriority(void);
extern int sys_getpriority(void);

static int (*syscalls[])(void) = {
    // ... existing syscalls ...
    [SYS_setpriority] sys_setpriority,
    [SYS_getpriority] sys_getpriority,
};
```

#### Step 4: Implement sys_setpriority and sys_getpriority in sysproc.c
In `kernel/sysproc.c`:
```c
int
sys_setpriority(void)
{
  int priority;
  if(argint(0, &priority) < 0)
    return -1;
  myproc()->priority = priority;
  return 0;
}

int
sys_getpriority(void)
{
  return myproc()->priority;
}
```

#### Step 5: Initialize priority in proc.c
In `kernel/proc.c`, in `allocproc()`:
```c
p->priority = 300;  // Default priority
```

#### Step 6: Modify scheduler() in proc.c
Replace the scheduler function:
```c
void
scheduler(void)
{
  struct proc *p;
  struct cpu *c = mycpu();
  c->proc = 0;

  for(;;){
    intr_on();

    struct proc *highest = 0;
    int max_priority = -1;

    // Pass 1: scan for the highest-priority runnable process, holding
    // only ONE process lock at a time (the old version kept the
    // incumbent's lock held across the rest of the scan while
    // acquiring further locks — two proc locks held at once, which
    // risks a lock-order deadlock against other kernel paths).
    for(p = proc; p < &proc[NPROC]; p++){
      acquire(&p->lock);
      if(p->state == RUNNABLE && p->priority > max_priority){
        max_priority = p->priority;
        highest = p;
      }
      release(&p->lock);
    }

    // Pass 2: re-acquire only the winner and re-check its state before
    // running it — it may have changed since the lock was released.
    if(highest){
      acquire(&highest->lock);
      if(highest->state == RUNNABLE){
        highest->state = RUNNING;
        c->proc = highest;
        swtch(&c->context, &highest->context);
        c->proc = 0;
      }
      release(&highest->lock);
    }
  }
}
```

#### Step 7: Add user system calls in user/usys.S
```asm
entry("setpriority");
entry("getpriority");
```

---

## Question B1: FCFS (First Come First Served) Scheduling

### Question Statement
Implement FCFS scheduling algorithm in xv6. The default RR scheduling will work only for processes with pid 1 (init) and 2 (shell). For all other processes, whichever comes first will run till completion.

### Given Code (testloop.c)
```c
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char* argv[]){
    int pid = getpid();
    int entry_time = uptime();
    uint32 iters = atoi(argv[1]);
    printf("Process %d: Starting %u iterations at time %d\n", pid, iters, entry_time);
    for(int i = 0; i < iters; i++){
        // do some dummy work
        for(int j = 0; j < 50000000; j++){
            int x = j * j;
            x = x + 1;
        }
    }
    int exit_time = uptime();
    printf("Process %d: Finished at time %d\n", pid, exit_time);
    exit(0);
}
```

### Solution

#### Step 1: Modify proc.h - Add arrival time tracking
In `kernel/proc.h`, add to `struct proc`:
```c
struct proc {
  // ... existing fields ...
  uint arrival_time;  // Track when process became runnable
  // ... existing fields ...
};
```

#### Step 2: Initialize arrival time in proc.c
In `kernel/proc.c`, in `allocproc()`:
```c
p->arrival_time = 0;
```

#### Step 3: Set arrival time in userinit() and fork()
In `kernel/proc.c`, when a process becomes runnable. **Note the original
draft of this step had a bug: it wrote `c->arrival_time = ticks;`, which
sets the field on the *cpu* struct (`c`), not on the new child process.
It should use the child process pointer (`np` in xv6's real `fork()`)
instead. Also read the global `ticks` under `tickslock` for safety:**
```c
// In fork(), right where the child is marked RUNNABLE, before releasing its lock:
acquire(&tickslock);
np->arrival_time = ticks;  // FIXED: was `c->arrival_time` (wrong struct entirely)
release(&tickslock);

// In userinit(), right where the init process is marked RUNNABLE:
acquire(&tickslock);
p->arrival_time = ticks;
release(&tickslock);
```

#### Step 4: Modify scheduler() in proc.c
Replace the scheduler function. **This version fixes two bugs in the
original draft:**
1. The old scan held the current winner's lock (`earliest->lock`)
   while continuing to acquire further locks later in the same loop —
   two proc locks held at once, risking a lock-order deadlock.
   `UINT_MAX` is also undefined in the xv6 kernel (no `<limits.h>`),
   so the original wouldn't compile; a `found` flag replaces it.
2. The original `else if` branch for pid 1/2 only ever released the
   lock and never actually scheduled them — meaning `init`/`sh` could
   starve completely whenever no `earliest` candidate existed. The fix
   adds an explicit fallback pass that runs pid 1/2 in normal RR when
   no other process is runnable.
```c
void
scheduler(void)
{
  struct proc *p;
  struct cpu *c = mycpu();
  c->proc = 0;

  for(;;){
    intr_on();

    struct proc *earliest = 0;
    uint earliest_time = 0;
    int found = 0;

    // Pass 1: find the earliest-arriving runnable process among
    // pid > 2, holding only one process lock at a time.
    for(p = proc; p < &proc[NPROC]; p++){
      acquire(&p->lock);
      if(p->state == RUNNABLE && p->pid > 2){
        if(!found || p->arrival_time < earliest_time){
          earliest_time = p->arrival_time;
          earliest = p;
          found = 1;
        }
      }
      release(&p->lock);
    }

    if(earliest){
      // Pass 2: re-acquire only the winner and re-check its state.
      acquire(&earliest->lock);
      if(earliest->state == RUNNABLE){
        earliest->state = RUNNING;
        c->proc = earliest;
        swtch(&c->context, &earliest->context);
        c->proc = 0;
      }
      release(&earliest->lock);
      continue;
    }

    // FIX: no pid > 2 process is runnable — fall back to normal RR
    // for init (pid 1) and the shell (pid 2). The original code had
    // no path that ever scheduled these, which could hang the system.
    for(p = proc; p < &proc[NPROC]; p++){
      acquire(&p->lock);
      if(p->state == RUNNABLE && (p->pid == 1 || p->pid == 2)){
        p->state = RUNNING;
        c->proc = p;
        swtch(&c->context, &p->context);
        c->proc = 0;
        release(&p->lock);
        break;
      }
      release(&p->lock);
    }
  }
}
```

---

## Question B2: Priority-Based Scheduling with Aging

### Question Statement
Implement priority-based scheduler with an aging mechanism. Default priority is 1000. If a process remains unscheduled for 30 ticks, its priority increases by 10. The waiting time should be reset whenever the process is scheduled or its priority is adjusted.

### Given Code (testloop.c)
```c
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define BYEL "\e[1;33m"
#define BRED "\e[1;31m"
#define CRESET "\e[0m"

int main(int argc, char* argv[]){
    int pid = getpid();
    uint32 iters = atoi(argv[1]);
    int priority = atoi(argv[2]);
    setpriority(priority);
    sleep(5);  // Let scheduler run
    int entry_time = uptime();
    printf(BYEL "PID %d: Starting %u iterations at time %d. Initial priority: %d, current: %d\n" CRESET, 
           pid, iters, entry_time, priority, getpriority());
    for(int i = 0; i < iters; i++){
        // do some dummy work
        for(int j = 0; j < 50000000; j++){
            int x = j * j;
            x = x + 1;
        }
    }
    int exit_time = uptime();
    printf(BRED "PID %d: Finished at time %d. Initial pr: %d, current: %d\n" CRESET, 
           pid, exit_time, priority, getpriority());
    exit(0);
}
```

### Solution

#### Step 1: Modify proc.h - Add aging fields
In `kernel/proc.h`, add to `struct proc`. **Note: the original draft used
a `waiting_time` counter incremented once per pass through the
scheduler's outer loop. That loop can run far more or less often than
one real timer tick — it re-runs every time a process yields, blocks,
or exits — so "30 ticks" was never actually measuring 30 ticks. The fix
below stores a timestamp (`last_sched_tick`) and compares it against
the kernel's real `ticks` counter instead:**
```c
struct proc {
  // ... existing fields ...
  int priority;           // Current priority
  int initial_priority;   // Initial priority set by user
  uint last_sched_tick;   // Tick value when last scheduled or aged
  // ... existing fields ...
};
```

#### Step 2: Add system calls in kernel/syscall.h
```c
#define SYS_setpriority 23
#define SYS_getpriority 24
```

#### Step 3: Implement system calls in kernel/syscall.c
```c
extern int sys_setpriority(void);
extern int sys_getpriority(void);

static int (*syscalls[])(void) = {
    // ... existing syscalls ...
    [SYS_setpriority] sys_setpriority,
    [SYS_getpriority] sys_getpriority,
};
```

#### Step 4: Implement sys_setpriority and sys_getpriority in sysproc.c
```c
uint64
sys_setpriority(void)
{
  int priority;
  if(argint(0, &priority) < 0)
    return -1;
  struct proc *p = myproc();
  p->priority = priority;
  p->initial_priority = priority;
  acquire(&tickslock);
  p->last_sched_tick = ticks;  // FIX: reset the wait clock, not a counter
  release(&tickslock);
  return 0;
}

uint64
sys_getpriority(void)
{
  return myproc()->priority;
}
```

#### Step 5: Initialize fields in proc.c allocproc()
```c
p->priority = 1000;           // Default priority
p->initial_priority = 1000;   // Initial priority
p->last_sched_tick = 0;       // Set properly once the process is made
                               // RUNNABLE in fork()/userinit() (see Step 6 note)
```

#### Step 6: Modify scheduler() in proc.c with aging
**Two bugs fixed here.** First, aging is now measured against the
kernel's real `ticks` counter instead of an increment-per-loop-pass
counter, so "30 ticks" means 30 actual timer ticks. Second, the
selection scan no longer holds the incumbent's lock while acquiring
further locks later in the same pass — it uses the same two-pass,
one-lock-at-a-time pattern as the other algorithms above.
```c
void
scheduler(void)
{
  struct proc *p;
  struct cpu *c = mycpu();
  c->proc = 0;

  for(;;){
    intr_on();

    acquire(&tickslock);
    uint now = ticks;
    release(&tickslock);

    // Aging: bump the priority of any RUNNABLE process that has gone
    // >= 30 real ticks since it last ran or was last aged.
    for(p = proc; p < &proc[NPROC]; p++){
      acquire(&p->lock);
      if(p->state == RUNNABLE && now - p->last_sched_tick >= 30){
        p->priority += 10;
        p->last_sched_tick = now;
        printf("Process %d priority increased to %d\n", p->pid, p->priority);
      }
      release(&p->lock);
    }

    struct proc *highest = 0;
    int max_priority = -1;

    // Pass 1: find the highest-priority runnable process, one lock
    // at a time (no lock held across the rest of the scan).
    for(p = proc; p < &proc[NPROC]; p++){
      acquire(&p->lock);
      if(p->state == RUNNABLE && p->priority > max_priority){
        max_priority = p->priority;
        highest = p;
      }
      release(&p->lock);
    }

    // Pass 2: re-acquire only the winner and re-check its state.
    if(highest){
      acquire(&highest->lock);
      if(highest->state == RUNNABLE){
        highest->state = RUNNING;
        highest->last_sched_tick = now;  // reset wait clock on schedule
        c->proc = highest;
        swtch(&c->context, &highest->context);
        c->proc = 0;
      }
      release(&highest->lock);
    }
  }
}
```

Also update Step 3 (in `fork()`/`userinit()`) to set `last_sched_tick =
ticks` (under `tickslock`) at the point each process is first marked
`RUNNABLE` — the same fix applied to `arrival_time` in B1 above.

#### Step 7: Add user system calls in user/usys.S
```asm
entry("setpriority");
entry("getpriority");
```

---

## Question C2: Longest Weighted Wait (LWW) Scheduling

### Question Statement
Replace the round-robin scheduler with one where waiting builds pressure. Every process starts with wait score 0. Before each scheduling decision, add each RUNNABLE process's priority to its score exactly once. Pick the RUNNABLE process with the largest resulting score; break ties by process-table order. Reset the selected process's score to 0, then run it until it yields, blocks, or exits.

### Specification Parameters
Define these macros in `kernel/param.h`:
```c
#define MIN_PRIORITY 1
#define MAX_PRIORITY 10
#define DEFAULT_PRIORITY 5
```

### System Calls
```c
int setpriority(int priority);  // Sets priority, returns 0 on success, -1 on invalid
int getpriority(void);           // Returns current priority
```

### Given Code (testloop.c)
```c
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define BYEL "\033[1;33m"
#define BRED "\033[1;31m"
#define CRESET "\033[0m"

int
main(int argc, char *argv[])
{
  if(argc != 3){
    fprintf(2, "Usage: testloop <iterations> <priority>\n");
    exit(1);
  }

  int iters = atoi(argv[1]);
  int priority = atoi(argv[2]);
  if(iters <= 0){
    fprintf(2, "testloop: iterations must be positive\n");
    exit(1);
  }
  if(setpriority(priority) < 0)
    fprintf(2, "testloop: invalid priority; using default\n");

  sleep(5);

  int pid = getpid();
  printf(BYEL "PID %d: starting %d iterations at time %d, priority %d\n" CRESET,
         pid, iters, uptime(), getpriority());

  for(int i = 0; i < iters; i++)
    for(volatile int j = 0; j < 20000000; j++)
      ;

  printf(BRED "PID %d: finished at time %d\n" CRESET, pid, uptime());
  exit(0);
}
```

### Solution

#### Step 1: Add parameter macros to kernel/param.h
In `kernel/param.h`, add:
```c
#define MIN_PRIORITY 1
#define MAX_PRIORITY 10
#define DEFAULT_PRIORITY 5
```

#### Step 2: Modify proc.h - Add priority and wait score fields
In `kernel/proc.h`, add to `struct proc`:
```c
struct proc {
  // ... existing fields ...
  int priority;    // Process priority (1-10)
  int waitscore;   // Accumulated wait score for scheduling
  // ... existing fields ...
};
```

#### Step 3: Add system calls in kernel/syscall.h
```c
#define SYS_setpriority 25
#define SYS_getpriority 26
```

#### Step 4: Implement system calls in kernel/syscall.c
```c
extern uint64 sys_setpriority(void);
extern uint64 sys_getpriority(void);

static uint64 (*syscalls[])(void) = {
    // ... existing syscalls ...
    [SYS_setpriority] sys_setpriority,
    [SYS_getpriority] sys_getpriority,
};
```

#### Step 5: Add sys_setpriority and sys_getpriority to kernel/sysproc.c
Append these to `kernel/sysproc.c`:
```c
uint64
sys_setpriority(void)
{
  int priority;
  argint(0, &priority);
  struct proc *p = myproc();
  if(priority < MIN_PRIORITY || priority > MAX_PRIORITY){
    p->priority = DEFAULT_PRIORITY;
    return -1;
  }
  p->priority = priority;
  return 0;
}

uint64
sys_getpriority(void)
{
  return myproc()->priority;
}
```

#### Step 6: Initialize fields in proc.c allocproc()
In `kernel/proc.c`, in the `allocproc()` function:
```c
p->priority = DEFAULT_PRIORITY;  // Initialize priority
p->waitscore = 0;                 // Initialize wait score
```

#### Step 7: Handle priority inheritance in fork()
In `kernel/proc.c`, in the `fork()` function when creating the child:
```c
// Copy parent's priority to child
np->priority = p->priority;
// But reset child's wait score to 0
np->waitscore = 0;
```

#### Step 8: Modify scheduler() in proc.c with LWW algorithm
Replace the scheduler function. **Fix applied:** Step 2's original scan
held the incumbent's lock (`chosen->lock`) while continuing to acquire
further locks for the rest of the scan — two proc locks held
simultaneously, which risks a lock-order deadlock. It's rewritten as a
two-pass, one-lock-at-a-time scan, then a re-acquire-and-recheck on the
winner only. Step 1's accumulation loop was already safe as written
(only ever one lock held), so it's unchanged. The strict `>`
comparison still gives table-order tie-breaking for free, matching the
spec.
```c
void
scheduler(void)
{
  struct proc *p;
  struct cpu *c = mycpu();
  c->proc = 0;

  for(;;){
    intr_on();

    // Step 1: add priority to wait score of every RUNNABLE process.
    // (Safe as-is: only one lock is ever held at a time.)
    for(p = proc; p < &proc[NPROC]; p++){
      acquire(&p->lock);
      if(p->state == RUNNABLE){
        p->waitscore += p->priority;
      }
      release(&p->lock);
    }

    // Step 2 (FIXED): scan for the highest wait score, holding only
    // ONE process lock at a time. Strict '>' preserves table-order
    // tie-breaking.
    struct proc *chosen = 0;
    int highest_score = -1;

    for(p = proc; p < &proc[NPROC]; p++){
      acquire(&p->lock);
      if(p->state == RUNNABLE && p->waitscore > highest_score){
        highest_score = p->waitscore;
        chosen = p;
      }
      release(&p->lock);
    }

    // Step 3 (FIXED): re-acquire only the winner's lock and re-check
    // it is still RUNNABLE — its state may have changed since Step 2
    // released the lock — before resetting its score and running it.
    if(chosen){
      acquire(&chosen->lock);
      if(chosen->state == RUNNABLE){
        // Optional: Log the selection (toggle this for debugging)
        // printf("PICK: Process %d chosen with wait %d at priority %d\n",
        //        chosen->pid, chosen->waitscore, chosen->priority);

        chosen->waitscore = 0;
        chosen->state = RUNNING;
        c->proc = chosen;
        swtch(&c->context, &chosen->context);
        c->proc = 0;
      }
      release(&chosen->lock);
    }
  }
}
```

#### Step 9: Add user system calls in user/usys.pl
Add these entries:
```perl
entry("setpriority");
entry("getpriority");
```

#### Step 10: Declare functions in user/user.h
```c
int setpriority(int);
int getpriority(void);
```

#### Step 11: Add testloop to Makefile
In the Makefile, add `$U/_testloop` to UPROGS:
```makefile
UPROGS=\
	$U/_cat\
	$U/_echo\
	...
	$U/_testloop\
```

And add the testloop.c compilation:
```makefile
$U/_testloop: $U/testloop.o $(ULIB)
	$(LD) $(LDFLAGS) -N -e main -Ttext 0 -o $U/_testloop $U/testloop.o -L$(U) -lusr
```

### Algorithm Explanation

The Longest Weighted Wait (LWW) scheduler works as follows:

1. **Score Accumulation**: Before each scheduling decision, every RUNNABLE process has its priority added to its wait score exactly once.

2. **Selection**: The process with the highest wait score is chosen. Ties are broken by process-table order (earliest in proc[] array wins).

3. **Score Reset**: The chosen process's wait score is reset to 0.

4. **Execution**: The selected process runs until it yields, blocks, or exits. There is no explicit time quantum - processes run to their next natural yield/block/exit point.

5. **Non-RUNNABLE**: Processes that are not RUNNABLE (sleeping, waiting for I/O, etc.) keep their scores unchanged and don't participate in selection.

### Example Trace

Given three processes with priorities 8, 4, and 2, starting with scores 0:

```
Selection 1: Add priorities → scores [8, 4, 2] → Pick P1 (score 8) → Reset P1 → scores [0, 4, 2]
Selection 2: Add priorities → scores [8, 8, 4] → Pick P1 (earlier in table) → Reset P1 → scores [0, 8, 4]
Selection 3: Add priorities → scores [8, 12, 6] → Pick P2 (score 12) → Reset P2 → scores [8, 0, 6]
Selection 4: Add priorities → scores [16, 4, 8] → Pick P1 (score 16) → Reset P1 → scores [0, 4, 8]
Selection 5: Add priorities → scores [8, 8, 10] → Pick P3 (score 10) → Reset P3 → scores [8, 8, 0]
```

---

## Common Implementation Notes

### For All Implementations:

1. **Modify user/usys.S** - Add these entries for each system call:
   ```asm
   entry("setlength");      // For A1
   entry("setpriority");    // For A2, B2
   entry("getpriority");    // For A2, B2
   ```

2. **Update kernel/Makefile** - Ensure:
   - CPUS := 1 (Run on single CPU)
   - All new source files are compiled

3. **Testing Instructions**:
   - Set CPUS := 1 in Makefile
   - Run commands one at a time in shell (not all at once)
   - Be quick providing inputs to match expected ordering

4. **Submission**:
   ```bash
   git add --all
   git diff HEAD > ../2005010.patch
   ```

### Key Differences Between Algorithms:

| Algorithm | Selection Criteria | Priority Range | Score Mechanism | Tie-Breaking |
|-----------|------------------|-----------------|-----------------|--------------|
| A1: SJF | Shortest job length | N/A | N/A | N/A |
| A2: Priority | Highest priority value | 0-infinite | No | Continuous |
| B1: FCFS | Earliest arrival time | N/A | No | Arrival order |
| B2: Priority + Aging | Highest priority (auto-increase) | 0-infinite | No | Waiting time |
| C2: LWW | Highest accumulated wait score | 1-10 (clamped) | score += priority each round | Process table order |

---

## Debugging Tips

1. **Print statements** - Add debug output to scheduler and system calls
2. **Timing issues** - Use `uptime()` to check actual execution times
3. **Lock management** - Ensure proper acquire/release of process locks
4. **Priority selection** - Verify correct process is chosen each scheduler run
5. **Edge cases** - Handle init and shell processes specially (B1)
6. **Aging logic** - Verify `last_sched_tick` comparisons and priority increments work correctly (B2)

---

## Bugs Fixed in This Revision

The first draft of this guide compiled conceptually but had real bugs.
Here's what was wrong and what changed:

| # | Where | Bug | Fix |
|---|-------|-----|-----|
| 1 | A1, A2, B1, B2, C2 schedulers | Selection scans held the current winner's `proc->lock` while continuing to `acquire()` further locks later in the same loop — two proc locks held at once, which risks a lock-order deadlock against other kernel code that locks two procs in the opposite order. | Rewritten as a two-pass scan: find the winner while holding only one lock at a time (release before moving on), then re-acquire *only* the winner's lock and re-check `state == RUNNABLE` before running it. |
| 2 | A1, B1 | Used `INT_MAX` / `UINT_MAX` as sentinels — neither is defined anywhere in the xv6 kernel (no `<limits.h>`), so this wouldn't compile. | Replaced with a `found` flag; no sentinel needed. |
| 3 | B1 `fork()` | `c->arrival_time = ticks;` set the field on the **cpu** struct (`c`), not the new child process — a plain wrong-variable bug. | Changed to `np->arrival_time = ticks;` (the child process pointer), read under `tickslock`. |
| 4 | B1 scheduler | The branch meant to let pid 1 (init) and pid 2 (shell) run in normal RR only ever released their lock — it never actually scheduled them. The system could hang if only init/shell were runnable. | Added an explicit fallback pass that schedules pid 1/2 in RR whenever no `pid > 2` process is runnable. |
| 5 | B2 scheduler | `waiting_time++` incremented once per pass through the scheduler's outer loop, not once per real timer tick. That loop reruns every time a process yields, blocks, or exits, so "ages after 30 ticks" never actually measured 30 ticks. | Replaced the counter with a timestamp field (`last_sched_tick`) compared against the kernel's real `ticks` global: `now - p->last_sched_tick >= 30`. |

---

## References

- xv6 source code: `/kernel/` directory
- Process structure: `kernel/proc.h`
- Scheduler: `kernel/proc.c` 
- System calls: `kernel/syscall.c`, `kernel/syscall.h`, `kernel/sysproc.c`
- User-space system call stubs: `user/usys.S`
