# xv6 Scheduler Implementation Guide — 22 Batch

## Overview
This guide provides solutions for implementing various scheduling algorithms in xv6 kernel. Each section covers a different scheduling strategy with detailed implementation steps.

**Covered Algorithms:**
- **A1**: Priority-Scaled Round Robin
- **A2**: Least-Usage Fair Scheduling
- **B1**: Foreground-First Round Robin
- **B2**: Budget Epochs
- **C1**: Strict Priority with Starvation Rescue
- **C2**: Longest Weighted Wait (LWW)

> **Locking rule used everywhere below:** every scheduler scan holds only
> ONE `proc->lock` at a time (acquire → check/copy → release before moving
> to the next slot), then re-acquires *only* the winner's lock and re-checks
> `state == RUNNABLE` before running it. Never hold a winner's lock while
> continuing to acquire further locks in the same scan — that risks a
> lock-order deadlock.

---

## Question A1: Priority-Scaled Round Robin

### Question Statement
Replace the round-robin scheduler so that how long a process's turn lasts depends on its priority — not the order turns are handed out in. Turns still cycle through the process table in a fixed order, exactly like plain round robin. A process's turn lasts `BASE_QUANTUM * priority` ticks instead of one fixed length for everyone. A turn still ends early if the process blocks (e.g. sleep) or exits before its quota is used up. Nobody is ever skipped, so nobody starves.

### Specification Parameters
Define these macros in `kernel/param.h`:
```c
#define BASE_QUANTUM     2  // Ticks a priority-1 process gets per turn
#define MIN_PRIORITY     1  // Lowest priority a process may request
#define MAX_PRIORITY    10  // Highest priority a process may request
#define DEFAULT_PRIORITY 3  // Priority every process starts with
```

### System Calls
```c
int setpriority(int priority);  // Returns 0, or -1 if out of range (resets to DEFAULT_PRIORITY)
int getpriority(void);           // Returns current priority
```
A `fork()` child inherits its parent's current priority.

### Logging
Toggleable log line, printed every time a process is picked for a fresh turn:
```
TURN: Process 5 (testloop) starts a new turn at priority 4 (quantum 8)
```

### Given Code (testloop.c)
```c
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define BYEL "\e[1;33m"
#define BRED "\e[1;31m"
#define CRESET "\e[0m"

int
main(int argc, char *argv[])
{
  if(argc < 3){
    printf("Usage: testloop <iterations> <priority>\n");
    exit(1);
  }

  int pid = getpid();
  uint32 iters = atoi(argv[1]);
  int priority = atoi(argv[2]);

  setpriority(priority);
  sleep(5); // let the scheduler pick up the new priority before we start timing

  int entry_time = uptime();
  printf(BYEL "PID %d: starting %u iterations at time %d, priority %d\n" CRESET,
         pid, iters, entry_time, getpriority());

  for(uint32 i = 0; i < iters; i++){
    for(int j = 0; j < 50000000; j++){
      int x = j * j;
      x = x + 1;
    }
  }

  int exit_time = uptime();
  printf(BRED "PID %d: finished at time %d\n" CRESET, pid, exit_time);
  exit(0);
}
```

### Solution

#### Step 1: Add parameter macros to kernel/param.h
In `kernel/param.h`, add:
```c
#define BASE_QUANTUM     2
#define MIN_PRIORITY     1
#define MAX_PRIORITY    10
#define DEFAULT_PRIORITY 3
```

#### Step 2: Modify proc.h - Add priority and accounting fields
In `kernel/proc.h`, add to `struct proc`:
```c
struct proc {
  // ... existing fields ...
  int priority;    // Scheduling priority (MIN_PRIORITY..MAX_PRIORITY)
  int ticks_run;   // Ticks consumed in the current turn
  // ... existing fields ...
};
```

#### Step 3: Add system calls in kernel/syscall.h
```c
#define SYS_setpriority 22
#define SYS_getpriority 23
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
  acquire(&p->lock);
  if(priority < MIN_PRIORITY || priority > MAX_PRIORITY){
    p->priority = DEFAULT_PRIORITY;
    release(&p->lock);
    return -1;
  }
  p->priority = priority;
  release(&p->lock);
  return 0;
}

uint64
sys_getpriority(void)
{
  struct proc *p = myproc();
  acquire(&p->lock);
  int pr = p->priority;
  release(&p->lock);
  return pr;
}
```

#### Step 6: Initialize fields in proc.c allocproc()
In `kernel/proc.c`, in `allocproc()`:
```c
p->priority = DEFAULT_PRIORITY;
p->ticks_run = 0;
```

#### Step 7: Handle priority inheritance in fork()
In `kernel/proc.c`, in `fork()` when creating the child:
```c
// Inherit priority from parent; fresh turn accounting.
np->priority = p->priority;
np->ticks_run = 0;
```

#### Step 8: Enforce the quantum in the timer path
The scheduler itself blocks in `swtch()`, so it cannot count ticks. Count
them where ticks happen — in `kernel/trap.c`, in `clockintr()` (or the
`yield()` call it triggers). Sketch:
```c
// In clockintr(), after updating ticks, before yield:
if(myproc() && myproc()->state == RUNNING){
  struct proc *p = myproc();
  // Called with no proc lock held here; protect with p->lock.
  acquire(&p->lock);
  p->ticks_run++;
  int quantum = BASE_QUANTUM * p->priority;
  int expired = (p->ticks_run >= quantum);
  if(expired)
    p->ticks_run = 0;
  release(&p->lock);
  if(expired)
    yield();
}
```
A turn still ends early on `sleep()`/`exit()` because those paths leave
`RUNNING` without waiting for the quantum to expire. Reset `ticks_run = 0`
whenever the scheduler picks a process for a fresh turn (next step), and
also when a process blocks, so a re-woken process starts a fresh turn.

#### Step 9: Modify scheduler() in proc.c — cyclic RR, scaled turns
Replace the scheduler function. It visits table slots in fixed order
(starting each cycle at `proc[0]`), runs each `RUNNABLE` process once with
a fresh `ticks_run`, and computes the quantum only for accounting/logging —
preemption itself happens in the timer path above:
```c
void
scheduler(void)
{
  struct proc *p;
  struct cpu *c = mycpu();
  c->proc = 0;

  for(;;){
    intr_on();

    // One full cycle: visit every slot in table order.
    for(p = proc; p < &proc[NPROC]; p++){
      acquire(&p->lock);
      if(p->state != RUNNABLE){
        release(&p->lock);
        continue;
      }
      // Fresh turn: reset accounting, snapshot priority for the quantum.
      p->ticks_run = 0;
      int pr = p->priority;
      p->state = RUNNING;
      c->proc = p;
      // Optional toggleable log (rebuild with 1 to enable):
      // printf("TURN: Process %d (%s) starts a new turn at priority %d (quantum %d)\n",
      //        p->pid, p->name, pr, BASE_QUANTUM * pr);
      swtch(&c->context, &p->context);
      c->proc = 0;
      release(&p->lock);
    }
  }
}
```
Note this holds exactly one proc lock across `swtch()` — the standard xv6
pattern (the running process releases it in `sched()` and re-acquires before
returning). The per-slot acquire/check/run/continue structure is what keeps
the order strictly cyclic: wakeups and priority changes never restart a scan.

#### Step 10: Declare functions in user/user.h
```c
int setpriority(int);
int getpriority(void);
```

#### Step 11: Add user stubs in user/usys.pl
```perl
entry("setpriority");
entry("getpriority");
```

#### Step 12: Add testloop to Makefile
Save the given `testloop.c` to `user/testloop.c`, add `$U/_testloop` to
`UPROGS`, and set `CPUS := 1`.

### Sample Session
Three equal-work processes, launched one line at a time (logging off):
```
$ testloop 60 8 &   # PID 4: starting 60 iterations at time 42, priority 8
$ testloop 60 4 &   # PID 6: starting 60 iterations at time 90, priority 4
$ testloop 60 2 &   # PID 8: starting 60 iterations at time 106, priority 2
PID 4: finished at time 98
PID 6: finished at time 152
PID 8: finished at time 181
```
Equal work, but priority 8 finishes well ahead of 4 and 2. PIDs and exact
times vary; ordering by priority share is what matters.

---

## Question A2: Least-Usage Fair Scheduling

### Question Statement
Replace round robin with one that always runs whoever has had the least CPU time so far, weighted by priority. Every process starts with usage score 0. To pick who runs next, choose the runnable process with the smallest usage score (ties broken by table order). Every time a process is picked and runs, add `(MAX_PRIORITY + 1 - priority)` to its usage score — a priority-10 process's score barely moves; a priority-1 process's jumps a lot. There is no fixed turn length or queue: the smallest-score-wins rule alone decides everything on every reschedule.

### Specification Parameters
Define these macros in `kernel/param.h`:
```c
#define MIN_PRIORITY     1
#define MAX_PRIORITY    10
#define DEFAULT_PRIORITY 5
```

### System Calls
```c
int setpriority(int priority);  // Returns 0, or -1 if out of range (resets to DEFAULT_PRIORITY)
int getpriority(void);           // Returns current priority
```
A `fork()` child inherits its parent's current priority but starts with usage score 0.

### Logging
Toggleable log line, printed every time the scheduler picks a process:
```
PICK: Process 5 (testloop) selected with usage 42 at priority 4
```

### Given Code (testloop.c)
Same shape as A1: `testloop <iterations> <priority>` calls
`setpriority(priority)`, sleeps 5 ticks, prints start/finish with
`uptime()` and `getpriority()`. (See A1 listing; only the scheduling policy
differs.)

### Solution

#### Step 1: Add parameter macros to kernel/param.h
```c
#define MIN_PRIORITY     1
#define MAX_PRIORITY    10
#define DEFAULT_PRIORITY 5
```

#### Step 2: Modify proc.h - Add priority and usage fields
```c
struct proc {
  // ... existing fields ...
  int priority;  // Scheduling priority (MIN_PRIORITY..MAX_PRIORITY)
  int usage;     // Weighted CPU usage score (starts 0)
  // ... existing fields ...
};
```

#### Step 3: Add system calls in kernel/syscall.h
```c
#define SYS_setpriority 22
#define SYS_getpriority 23
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
```c
uint64
sys_setpriority(void)
{
  int priority;
  argint(0, &priority);
  struct proc *p = myproc();
  acquire(&p->lock);
  if(priority < MIN_PRIORITY || priority > MAX_PRIORITY){
    p->priority = DEFAULT_PRIORITY;
    release(&p->lock);
    return -1;
  }
  p->priority = priority;
  // NOTE: usage is NOT reset here — only fork() zeroes it.
  release(&p->lock);
  return 0;
}

uint64
sys_getpriority(void)
{
  struct proc *p = myproc();
  acquire(&p->lock);
  int pr = p->priority;
  release(&p->lock);
  return pr;
}
```

#### Step 6: Initialize fields in proc.c allocproc()
```c
p->priority = DEFAULT_PRIORITY;
p->usage = 0;
```

#### Step 7: Handle inheritance in fork()
```c
// Child inherits priority but starts with a clean usage score.
np->priority = p->priority;
np->usage = 0;
```

#### Step 8: Modify scheduler() in proc.c
```c
void
scheduler(void)
{
  struct proc *p;
  struct cpu *c = mycpu();
  c->proc = 0;

  for(;;){
    intr_on();

    // Pass 1: find the RUNNABLE process with the smallest usage.
    // One lock at a time; strict '<' keeps table-order tie-breaking.
    struct proc *chosen = 0;
    int best = 0;
    int found = 0;

    for(p = proc; p < &proc[NPROC]; p++){
      acquire(&p->lock);
      if(p->state == RUNNABLE && (!found || p->usage < best)){
        best = p->usage;
        chosen = p;
        found = 1;
      }
      release(&p->lock);
    }

    // Pass 2: re-acquire only the winner, re-check, charge, run.
    if(chosen){
      acquire(&chosen->lock);
      if(chosen->state == RUNNABLE){
        // Optional log (value BEFORE charging):
        // printf("PICK: Process %d (%s) selected with usage %d at priority %d\n",
        //        chosen->pid, chosen->name, chosen->usage, chosen->priority);
        chosen->usage += (MAX_PRIORITY + 1 - chosen->priority);
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

#### Step 9: Wire up user space (user.h, usys.pl, Makefile)
Same as A1 Steps 10–12: declare `setpriority`/`getpriority`, add the
`usys.pl` entries, save `testloop.c`, add `$U/_testloop` to `UPROGS`, set
`CPUS := 1`.

### Algorithm Explanation
Each pick raises the winner's usage by `(11 - priority)`: priority 8 pays
only 3 per run, priority 4 pays 7, priority 2 pays 9. High-priority
processes therefore stay at low usage and get picked again sooner, while
low-priority processes price themselves out for longer. No quantum, no
queue, no starvation — usage grows monotonically so everyone eventually
becomes the minimum again.

### Sample Session
```
$ testloop 60 8 &   # PID 4: starting at time 42, priority 8
$ testloop 60 4 &   # PID 6: starting at time 60, priority 4
$ testloop 60 2 &   # PID 8: starting at time 74, priority 2
PID 4: finished at time 122
PID 6: finished at time 164
PID 8: finished at time 175
```

---

## Question B1: Foreground-First Round Robin

### Question Statement
Modify the scheduler so that each cycle visits foreground processes first, then background processes. Each cycle consists of two complete process-table scans. A process is foreground if `priority >= FG_THRESHOLD`; otherwise it is background. Priority has no other effect within either class. First scan: visit slots from beginning to end and run each RUNNABLE foreground process once. Second scan: start again at the beginning and run each RUNNABLE background process once. Then repeat. Start the background scan even if foreground processes remain runnable. A selected process runs until it yields, blocks, or exits. On return, continue with the following table slot. Check state and class when visiting each slot; wakeups and priority changes do not restart a scan. Skip sleeping/unused slots. An empty class simply completes its scan without running anything. With fixed priorities, each continuously runnable process gets one selection per cycle; foreground means earlier service, not extra CPU share.

### Specification Parameters
Define these macros in `kernel/param.h`:
```c
#define MIN_PRIORITY     1
#define MAX_PRIORITY    10
#define DEFAULT_PRIORITY 5
#define FG_THRESHOLD     5  // Priority at or above this is foreground
```

### System Calls
```c
int setpriority(int priority);  // Returns 0; invalid input resets to DEFAULT_PRIORITY, returns -1
int getpriority(void);           // Returns current priority
```
A `fork()` child inherits its parent's priority. A changed priority decides the class when the scheduler next visits that slot. The syscall bodies are supplied in `syspriority.c` (see Step 5).

### Logging
Toggleable log line for every selection, showing the class:
```
PICK: Process 5 (testloop) from FOREGROUND at priority 8
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
```c
#define MIN_PRIORITY     1
#define MAX_PRIORITY    10
#define DEFAULT_PRIORITY 5
#define FG_THRESHOLD     5
```

#### Step 2: Modify proc.h - Add priority field
```c
struct proc {
  // ... existing fields ...
  int priority;  // Scheduling priority (MIN_PRIORITY..MAX_PRIORITY)
  // ... existing fields ...
};
```

#### Step 3: Add system calls in kernel/syscall.h
```c
#define SYS_setpriority 22
#define SYS_getpriority 23
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

#### Step 5: Append supplied syspriority.c to kernel/sysproc.c
The supplied bodies (append verbatim to `kernel/sysproc.c`):
```c
uint64
sys_setpriority(void)
{
  int priority;
  int result = 0;
  argint(0, &priority);
  if(priority < MIN_PRIORITY || priority > MAX_PRIORITY){
    priority = DEFAULT_PRIORITY;
    result = -1;
  }

  struct proc *p = myproc();
  acquire(&p->lock);
  p->priority = priority;
  release(&p->lock);
  return result;
}

uint64
sys_getpriority(void)
{
  return myproc()->priority;
}
```
> Note: `sys_getpriority` reads `p->priority` without the lock in the
> supplied file. It works for the assignment, but a strict version wraps
> the read in `acquire(&p->lock)` / `release(&p->lock)` like A1 Step 5.

#### Step 6: Initialize priority in proc.c allocproc()
```c
p->priority = DEFAULT_PRIORITY;
```

#### Step 7: Handle priority inheritance in fork()
```c
np->priority = p->priority;
```

#### Step 8: Modify scheduler() in proc.c — two scans per cycle
```c
void
scheduler(void)
{
  struct proc *p;
  struct cpu *c = mycpu();
  c->proc = 0;

  for(;;){
    intr_on();

    // Scan 1: foreground (priority >= FG_THRESHOLD), table order.
    for(p = proc; p < &proc[NPROC]; p++){
      acquire(&p->lock);
      if(p->state != RUNNABLE || p->priority < FG_THRESHOLD){
        release(&p->lock);
        continue;
      }
      // Optional log:
      // printf("PICK: Process %d (%s) from FOREGROUND at priority %d\n",
      //        p->pid, p->name, p->priority);
      p->state = RUNNING;
      c->proc = p;
      swtch(&c->context, &p->context);
      c->proc = 0;
      release(&p->lock);
    }

    // Scan 2: background (priority < FG_THRESHOLD), table order.
    // ALWAYS runs, even if foreground processes are still runnable.
    for(p = proc; p < &proc[NPROC]; p++){
      acquire(&p->lock);
      if(p->state != RUNNABLE || p->priority >= FG_THRESHOLD){
        release(&p->lock);
        continue;
      }
      // Optional log:
      // printf("PICK: Process %d (%s) from BACKGROUND at priority %d\n",
      //        p->pid, p->name, p->priority);
      p->state = RUNNING;
      c->proc = p;
      swtch(&c->context, &p->context);
      c->proc = 0;
      release(&p->lock);
    }
  }
}
```
State and class are re-checked at each slot visit, so a process that slept
is skipped and a process whose priority changed mid-cycle is classified by
its current value. Each scan holds one lock across `swtch()` per the normal
xv6 pattern, never two at once.

#### Step 9: Wire up user space (user.h, usys.pl, Makefile)
Declare `setpriority`/`getpriority` in `user/user.h`, add both entries to
`user/usys.pl`, save `testloop.c` to `user/testloop.c`, add `$U/_testloop`
to `UPROGS`, set `CPUS := 1`.

### Deterministic Check
Table order P1, P2, P3, P4 with priorities 2, 8, 4, 5, all runnable and
fixed: each cycle selects **P2, P4, P1, P3** — P2/P4 in the foreground scan,
P1/P3 in the background scan.

### Sample Session
```
$ testloop 60 8 &   # PID 4: starting at time 8, priority 8 (foreground)
$ testloop 60 4 &   # PID 6: starting at time 15, priority 4 (background)
$ testloop 60 2 &   # PID 8: starting at time 22, priority 2 (background)
PID 4: finished at time 107
PID 6: finished at time 121
PID 8: finished at time 123
```

---

## Question B2: Budget Epochs

### Question Statement
Replace round robin with one that gives each process a budget of selections to run, then refills budgets at the start of a new epoch. Every process starts with a full budget: `BUDGET_UNIT × priority`. Pick the RUNNABLE process with the largest positive remaining budget; break ties by process-table order. A process with budget 0 is ineligible. Subtract one unit from the selected process's budget before running it. It runs until it yields, blocks, or exits; even an early block costs that one unit. If at least one process is RUNNABLE and all RUNNABLE processes have budget 0, start a new epoch: refill every process's budget to `BUDGET_UNIT × its own priority`, including sleeping processes. Then select again. If no process is RUNNABLE, wait for work without refilling. Sleeping processes otherwise keep their remaining budgets unchanged. Budget counts selections, not timer ticks.

### Specification Parameters
Define these macros in `kernel/param.h`:
```c
#define MIN_PRIORITY     1
#define MAX_PRIORITY    10
#define DEFAULT_PRIORITY 5
#define BUDGET_UNIT      2  // Selections granted per unit of priority
```

### System Calls
```c
int setpriority(int priority);  // Sets priority AND refills budget; 0 on success, -1 + reset to DEFAULT on invalid
int getpriority(void);           // Returns current priority
```
A `fork()` child inherits its parent's priority but starts with a full budget, not the parent's remaining budget. The syscall bodies are supplied in `syspriority.c` (see Step 5).

### Logging
Toggleable log line for every selection, showing the budget before subtracting one:
```
PICK: Process 5 (testloop) runs with budget 8 at priority 4
```

### Given Code (testloop.c)
Identical to B1's `testloop.c` (`testloop <iterations> <priority>`, validates input, `sleep(5)`, prints start/finish).

### Solution

#### Step 1: Add parameter macros to kernel/param.h
```c
#define MIN_PRIORITY     1
#define MAX_PRIORITY    10
#define DEFAULT_PRIORITY 5
#define BUDGET_UNIT      2
```

#### Step 2: Modify proc.h - Add priority and budget fields
```c
struct proc {
  // ... existing fields ...
  int priority;  // Scheduling priority (MIN_PRIORITY..MAX_PRIORITY)
  int budget;    // Remaining selections in this epoch
  // ... existing fields ...
};
```

#### Step 3: Add system calls in kernel/syscall.h
```c
#define SYS_setpriority 22
#define SYS_getpriority 23
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

#### Step 5: Append supplied syspriority.c to kernel/sysproc.c
Supplied bodies (append verbatim):
```c
uint64
sys_setpriority(void)
{
  int priority;
  int result = 0;
  argint(0, &priority);
  if(priority < MIN_PRIORITY || priority > MAX_PRIORITY){
    priority = DEFAULT_PRIORITY;
    result = -1;
  }

  struct proc *p = myproc();
  acquire(&p->lock);
  p->priority = priority;
  p->budget = priority * BUDGET_UNIT;
  release(&p->lock);
  return result;
}

uint64
sys_getpriority(void)
{
  return myproc()->priority;
}
```
Key point: `setpriority` refills the budget even when the priority is unchanged, and on invalid input it resets to `DEFAULT_PRIORITY` *and* refills.

#### Step 6: Initialize fields in proc.c allocproc()
```c
p->priority = DEFAULT_PRIORITY;
p->budget = DEFAULT_PRIORITY * BUDGET_UNIT;
```

#### Step 7: Handle inheritance in fork()
```c
// Inherit priority, but grant a FULL budget — not the parent's remainder.
np->priority = p->priority;
np->budget = p->priority * BUDGET_UNIT;
```

#### Step 8: Modify scheduler() in proc.c — largest budget wins, refill epochs
```c
void
scheduler(void)
{
  struct proc *p;
  struct cpu *c = mycpu();
  c->proc = 0;

  for(;;){
    intr_on();

    struct proc *chosen = 0;
    int best = 0;
    int any_runnable = 0;

    // Pass 1: largest positive budget among RUNNABLE, one lock at a time.
    // Strict '>' keeps table-order tie-breaking.
    for(p = proc; p < &proc[NPROC]; p++){
      acquire(&p->lock);
      if(p->state == RUNNABLE){
        any_runnable = 1;
        if(p->budget > 0 && (chosen == 0 || p->budget > best)){
          best = p->budget;
          chosen = p;
        }
      }
      release(&p->lock);
    }

    // Epoch refill: someone is runnable but every runnable budget is 0.
    // Refill EVERY process (including sleepers) to BUDGET_UNIT * own priority.
    if(any_runnable && chosen == 0){
      for(p = proc; p < &proc[NPROC]; p++){
        acquire(&p->lock);
        p->budget = p->priority * BUDGET_UNIT;
        if(p->state == RUNNABLE && (chosen == 0 || p->budget > best)){
          best = p->budget;
          chosen = p;
        }
        release(&p->lock);
      }
    }

    // Pass 2: charge one unit, then run.
    if(chosen){
      acquire(&chosen->lock);
      if(chosen->state == RUNNABLE && chosen->budget > 0){
        // Optional log (budget BEFORE subtracting):
        // printf("PICK: Process %d (%s) runs with budget %d at priority %d\n",
        //        chosen->pid, chosen->name, chosen->budget, chosen->priority);
        chosen->budget -= 1;
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
If nothing is RUNNABLE, neither pass picks anyone and no refill happens —
the scheduler simply loops and waits for work. An early block still costs
the one unit because the charge happens before `swtch()`.

#### Step 9: Wire up user space (user.h, usys.pl, Makefile)
Same as B1 Step 9.

### Deterministic Check
Only P1, P2 continuously runnable (table order), priorities 2 and 1, full budgets 4 and 2. First six selections: **P1, P1, P1, P2, P1, P2**. Both budgets are then 0; refill to 4 and 2 before the seventh selection, which picks P1.

### Sample Session
```
$ testloop 60 8 &   # PID 4: starting at time 8, priority 8
$ testloop 60 4 &   # PID 6: starting at time 20, priority 4
$ testloop 60 2 &   # PID 8: starting at time 42, priority 2
PID 4: finished at time 64
PID 6: finished at time 101
PID 8: finished at time 124
```

---

## Question C1: Strict Priority with Starvation Rescue

### Question Statement
Replace round robin with strict priority, plus a rescue rule that lets processes run even while higher-priority processes remain runnable. Every process starts with wait counter 0. Before each scheduling decision, increment every RUNNABLE process's counter by exactly one, regardless of priority. If any RUNNABLE process has counter >= RESCUE_THRESH, rescue the one with the largest counter, regardless of priority. Break equal-counter ties by process-table order. Otherwise, pick the RUNNABLE process with the highest priority. Break equal-priority ties by process-table order. Reset the selected process's counter to 0, whichever rule selected it. Run it until it yields, blocks, or exits; repeat at the next decision. Non-RUNNABLE processes keep counters unchanged. If none are runnable, wait. Counters count scheduling decisions, not timer ticks.

### Specification Parameters
Define these macros in `kernel/param.h`:
```c
#define MIN_PRIORITY     1
#define MAX_PRIORITY    10
#define DEFAULT_PRIORITY 5
#define RESCUE_THRESH   20  // Counter value that makes a process eligible for rescue
```

### System Calls
```c
int setpriority(int priority);  // Returns 0; invalid resets to DEFAULT_PRIORITY, returns -1; never resets the counter
int getpriority(void);           // Returns current priority
```
A `fork()` child inherits its parent's priority but starts with counter 0. The syscall bodies are supplied in `syspriority.c` (see Step 5).

### Logging
Toggleable log line for every rescue, showing the counter before resetting:
```
RESCUE: Process 4 (testloop) chosen with wait 20 at priority 2
```

### Given Code (testloop.c)
Identical to B1's `testloop.c` (`testloop <iterations> <priority>`, validates input, `sleep(5)`, prints start/finish).

### Solution

#### Step 1: Add parameter macros to kernel/param.h
```c
#define MIN_PRIORITY     1
#define MAX_PRIORITY    10
#define DEFAULT_PRIORITY 5
#define RESCUE_THRESH   20
```

#### Step 2: Modify proc.h - Add priority and wait-counter fields
```c
struct proc {
  // ... existing fields ...
  int priority;      // Scheduling priority (MIN_PRIORITY..MAX_PRIORITY)
  int waitcounter;   // Scheduling decisions since last selected
  // ... existing fields ...
};
```

#### Step 3: Add system calls in kernel/syscall.h
```c
#define SYS_setpriority 22
#define SYS_getpriority 23
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

#### Step 5: Append supplied syspriority.c to kernel/sysproc.c
Supplied bodies (append verbatim):
```c
uint64
sys_setpriority(void)
{
  int priority;
  int result = 0;
  argint(0, &priority);
  if(priority < MIN_PRIORITY || priority > MAX_PRIORITY){
    priority = DEFAULT_PRIORITY;
    result = -1;
  }

  struct proc *p = myproc();
  acquire(&p->lock);
  p->priority = priority;
  // NOTE: waitcounter is deliberately NOT reset here.
  release(&p->lock);
  return result;
}

uint64
sys_getpriority(void)
{
  return myproc()->priority;
}
```

#### Step 6: Initialize fields in proc.c allocproc()
```c
p->priority = DEFAULT_PRIORITY;
p->waitcounter = 0;
```

#### Step 7: Handle inheritance in fork()
```c
// Inherit priority; counter always starts fresh.
np->priority = p->priority;
np->waitcounter = 0;
```

#### Step 8: Modify scheduler() in proc.c — rescue first, priority otherwise
```c
void
scheduler(void)
{
  struct proc *p;
  struct cpu *c = mycpu();
  c->proc = 0;

  for(;;){
    intr_on();

    // Step 1: every RUNNABLE process's counter += 1, exactly once.
    // One lock at a time.
    for(p = proc; p < &proc[NPROC]; p++){
      acquire(&p->lock);
      if(p->state == RUNNABLE)
        p->waitcounter += 1;
      release(&p->lock);
    }

    // Step 2: rescue candidate — largest counter >= RESCUE_THRESH.
    // Strict '>' keeps table-order tie-breaking.
    struct proc *rescued = 0;
    int bestwait = 0;

    for(p = proc; p < &proc[NPROC]; p++){
      acquire(&p->lock);
      if(p->state == RUNNABLE && p->waitcounter >= RESCUE_THRESH &&
         (rescued == 0 || p->waitcounter > bestwait)){
        bestwait = p->waitcounter;
        rescued = p;
      }
      release(&p->lock);
    }

    // Step 3: otherwise highest priority (table order on ties).
    struct proc *chosen = rescued;
    if(chosen == 0){
      int bestpr = -1;
      for(p = proc; p < &proc[NPROC]; p++){
        acquire(&p->lock);
        if(p->state == RUNNABLE && p->priority > bestpr){
          bestpr = p->priority;
          chosen = p;
        }
        release(&p->lock);
      }
    }

    // Step 4: reset the winner's counter and run it.
    if(chosen){
      acquire(&chosen->lock);
      if(chosen->state == RUNNABLE){
        int was_rescue = (chosen == rescued);
        // Optional log, rescue picks only:
        // if(was_rescue)
        //   printf("RESCUE: Process %d (%s) chosen with wait %d at priority %d\n",
        //          chosen->pid, chosen->name, chosen->waitcounter, chosen->priority);
        (void)was_rescue;
        chosen->waitcounter = 0;
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
Note: the winner is re-checked after re-acquiring its lock — it may have
slept or exited between the scan and now. Equal priorities do NOT share the
CPU here: the earlier table slot wins every priority race until rescue fires.

#### Step 9: Wire up user space (user.h, usys.pl, Makefile)
Same as B1 Step 9.

### Deterministic Check
Only P1, P2 continuously runnable (table order), priorities 8 and 2,
counters 0. Decisions 1–19 select P1 (strict priority). At decision 20,
P2's counter reaches 20 and P2 is rescued. Decision 21 selects P1 again.
Reaching the threshold makes a process *eligible*; it may wait longer if
others have larger counters.

### Sample Session
Launched lowest priority first (logging off):
```
$ testloop 60 2 &   # PID 4: starting at time 7, priority 2
$ testloop 60 4 &   # PID 6: starting at time 13, priority 4
$ testloop 60 8 &   # PID 8: starting at time 19, priority 8
PID 8: finished at time 65
PID 6: finished at time 98
PID 4: finished at time 129
```
Finishing order alone does not demonstrate rescue — enable the log to
observe rescue selections.

---

## Question C2: Longest Weighted Wait (LWW) Scheduling

### Question Statement
Replace the round-robin scheduler with one where waiting builds pressure. Every process starts with wait score 0. Before each scheduling decision, add each RUNNABLE process's priority to its score exactly once. Pick the RUNNABLE process with the largest resulting score; break ties by process-table order. Reset the selected process's score to 0, then run it until it yields, blocks, or exits.

### Specification Parameters
Define these macros in `kernel/param.h`:
```c
#define MIN_PRIORITY     1
#define MAX_PRIORITY    10
#define DEFAULT_PRIORITY 5
```

### System Calls
```c
int setpriority(int priority);  // Sets priority, returns 0 on success, -1 on invalid (resets to DEFAULT)
int getpriority(void);           // Returns current priority
```
`setpriority` does not change the wait score. A `fork()` child inherits its parent's priority but starts with score 0. The syscall bodies are supplied in `syspriority.c` (see Step 5); the full reference diff is also supplied as `sectionc.patch`.

### Logging
Toggleable log line for every selection, showing the score before resetting:
```
PICK: Process 4 (testloop) chosen with wait 16 at priority 8
```
The reference patch implements this as a `SCHED_LOG` flag in `param.h`:
```c
#define SCHED_LOG 1  // set to 1 and rebuild to log scheduler picks
```

### Given Code (testloop.c)
Identical to B1's `testloop.c` (`testloop <iterations> <priority>`, validates input, `sleep(5)`, prints start/finish).

### Solution

#### Step 1: Add parameter macros to kernel/param.h
In `kernel/param.h`, add:
```c
#define MIN_PRIORITY      1
#define MAX_PRIORITY     10
#define DEFAULT_PRIORITY  5

#define SCHED_LOG         1  // set to 1 and rebuild to log scheduler picks
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
#define SYS_setpriority 22
#define SYS_getpriority 23
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

#### Step 5: Append supplied syspriority.c to kernel/sysproc.c
Supplied bodies (append verbatim):
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
> Locking note: unlike the B1/B2/C1 supplied files, this one touches
> `p->priority` with no lock held. It matches the reference patch exactly,
> but the safer form wraps both functions in
> `acquire(&p->lock)` / `release(&p->lock)` (and snapshots the value in
> `getpriority` before returning). Either passes; prefer the locked form if
> your examiner checks for it.

#### Step 6: Initialize fields in proc.c allocproc()
In `kernel/proc.c`, in `allocproc()`:
```c
p->priority = DEFAULT_PRIORITY;
p->waitscore = 0;
```

#### Step 7: Handle priority inheritance in fork()
In `kernel/proc.c`, in `fork()`:
```c
// Inherit priority from parent; wait score always starts fresh.
np->priority = p->priority;
np->waitscore = 0;
```

#### Step 8: Modify scheduler() in proc.c with LWW algorithm
Replace the scheduler function (this is the reference `sectionc.patch`
scheduler, with the two-pass locking made explicit):
```c
void
scheduler(void)
{
  struct proc *p;
  struct cpu *c = mycpu();
  c->proc = 0;

  for(;;){
    intr_on();

    // Step 1: add priority to wait score of every RUNNABLE process,
    // exactly once. (One lock at a time.)
    for(p = proc; p < &proc[NPROC]; p++){
      acquire(&p->lock);
      if(p->state == RUNNABLE)
        p->waitscore += p->priority;
      release(&p->lock);
    }

    // Step 2: scan for the highest wait score.
    // Strict '>' preserves table-order tie-breaking.
    struct proc *chosen = 0;
    int bestscore = -1;

    for(p = proc; p < &proc[NPROC]; p++){
      acquire(&p->lock);
      if(p->state == RUNNABLE && p->waitscore > bestscore){
        bestscore = p->waitscore;
        chosen = p;
      }
      release(&p->lock);
    }

    // Step 3: re-acquire only the winner, re-check, reset score, run.
    if(chosen){
      acquire(&chosen->lock);
      if(chosen->state == RUNNABLE){
#if SCHED_LOG
        printf("PICK: Process %d (%s) chosen with wait %d at priority %d\n",
               chosen->pid, chosen->name, chosen->waitscore, chosen->priority);
#endif
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
Save the given `testloop.c` to `user/testloop.c`, add `$U/_testloop` to `UPROGS`, set `CPUS := 1`. (The reference patch also fixes an unrelated `usertests.c` typo — `rwsbrk()` → `rwsbrk(char* s)` — needed to keep `-Werror` builds compiling.)

### Algorithm Explanation

The Longest Weighted Wait (LWW) scheduler works as follows:

1. **Score Accumulation**: Before each scheduling decision, every RUNNABLE process has its priority added to its wait score exactly once.

2. **Selection**: The process with the highest wait score is chosen. Ties are broken by process-table order (earliest in proc[] array wins).

3. **Score Reset**: The chosen process's wait score is reset to 0.

4. **Execution**: The selected process runs until it yields, blocks, or exits. There is no explicit time quantum.

5. **Non-RUNNABLE**: Processes that are not RUNNABLE keep their scores unchanged and don't participate in selection.

### Example Trace

Given three processes with priorities 8, 4, and 2, starting with scores 0:

```
Selection 1: Add priorities → scores [8, 4, 2] → Pick P1 (score 8) → Reset P1 → scores [0, 4, 2]
Selection 2: Add priorities → scores [8, 8, 4] → Pick P1 (earlier in table) → Reset P1 → scores [0, 8, 4]
Selection 3: Add priorities → scores [8, 12, 6] → Pick P2 (score 12) → Reset P2 → scores [8, 0, 6]
Selection 4: Add priorities → scores [16, 4, 8] → Pick P1 (score 16) → Reset P1 → scores [0, 4, 8]
Selection 5: Add priorities → scores [8, 8, 10] → Pick P3 (score 10) → Reset P3 → scores [8, 8, 0]
```

### Deterministic Check
Only P1, P2, P3 continuously runnable (table order), priorities 8, 4, 2, scores 0. First five selections: **P1, P1, P2, P1, P3**. The second selection is a table-order tie. Higher priority builds score faster, but the policy does not guarantee CPU shares proportional to priority — use widely spaced priorities since close values (e.g. 9 vs 7) can give identical shares.

### Sample Session
```
$ testloop 60 8 &   # PID 4: starting at time 38, priority 8
$ testloop 60 4 &   # PID 6: starting at time 55, priority 4
$ testloop 60 2 &   # PID 8: starting at time 72, priority 2
PID 4: finished at time 92
PID 6: finished at time 132
PID 8: finished at time 155
```

---

## Common Implementation Notes

### For All Implementations:

1. **Syscall plumbing** — every question needs all four touch points:
   - `kernel/syscall.h`: `#define SYS_setpriority 22` / `#define SYS_getpriority 23`
   - `kernel/syscall.c`: `extern` declarations + table entries
   - `user/usys.pl`: `entry("setpriority");` / `entry("getpriority");`
   - `user/user.h`: `int setpriority(int);` / `int getpriority(void);`

2. **Update Makefile** - Ensure:
   - CPUS := 1 (run on single CPU, otherwise scheduling order is meaningless)
   - `$U/_testloop` added to UPROGS and `user/testloop.c` saved as-is

3. **allocproc()/fork() discipline**:
   - `allocproc()`: always init new fields (`priority = DEFAULT_PRIORITY`; usage/score/counter/budget zeroed or set to full as specified)
   - `fork()`: child inherits `priority` (A1 default 3, all others default 5); usage/score/counter start at 0; B2 budget starts FULL (`priority * BUDGET_UNIT`)

4. **setpriority() semantics differ per question** — don't mix them up:
   - A1/A2: invalid → reset to DEFAULT, return -1; usage/counter untouched (A2)
   - B1/C1: invalid → reset to DEFAULT, return -1; C1 never touches the wait counter
   - B2: invalid → reset to DEFAULT **and refill budget**, return -1; valid calls refill too
   - C2: invalid → reset to DEFAULT, return -1; wait score untouched

5. **Testing Instructions**:
   - Set CPUS := 1 in Makefile
   - Launch with `&` one line at a time so processes overlap while running
   - Keep the toggleable log OFF for the sample-session output, ON to prove the mechanism (TURN/PICK/RESCUE lines)

6. **Submission**:
   ```bash
   git add --all
   git diff HEAD > ../STUDENT_ID.patch
   ```

### Key Differences Between Algorithms:

| Algorithm | Selection Criteria | Priority Range / Default | State per proc | Tie-Breaking |
|-----------|------------------|--------------------------|----------------|--------------|
| A1: Scaled RR | Table order; turn length = BASE_QUANTUM × priority | 1–10 / 3 | priority, ticks_run | Table order (no skipping) |
| A2: Least-Usage | Smallest usage; charge (11 − priority) | 1–10 / 5 | priority, usage | Table order |
| B1: Foreground-First | Foreground scan, then background scan | 1–10 / 5, FG≥5 | priority | Table order within each scan |
| B2: Budget Epochs | Largest positive budget; refill all on epoch | 1–10 / 5, unit 2 | priority, budget | Table order |
| C1: Strict + Rescue | Rescue (counter ≥ 20) else highest priority | 1–10 / 5, thresh 20 | priority, waitcounter | Table order in both rules |
| C2: LWW | Highest wait score (score += priority each round) | 1–10 / 5 | priority, waitscore | Table order |

---

## Debugging Tips

1. **Print statements** — enable the per-question log line (TURN / PICK / RESCUE) and rebuild; disable for final timing runs
2. **Timing issues** — use `uptime()` to check actual execution order; PIDs and timestamps vary under QEMU, only the policy pattern is graded
3. **Lock management** — one proc lock at a time in scans; re-check `state == RUNNABLE` after re-acquiring the winner
4. **Quantum bugs (A1)** — if every process gets equal CPU, the timer path isn't yielding on `ticks_run >= BASE_QUANTUM * priority`; if a process never runs, a scan is skipping it instead of cycling
5. **Epoch bugs (B2)** — refilling when nothing is RUNNABLE, or forgetting to include sleepers in the refill, both break the deterministic check
6. **Rescue bugs (C1)** — incrementing non-RUNNABLE counters, or resetting the counter in `setpriority`, both break the 20-decision check
7. **Score bugs (C2)** — adding priority more than once per decision, or resetting a loser's score, breaks the P1,P1,P2,P1,P3 trace

---

## References

- xv6 source code: `/kernel/` directory
- Process structure: `kernel/proc.h`
- Scheduler: `kernel/proc.c`
- Timer/preemption path: `kernel/trap.c` (A1 quantum enforcement)
- System calls: `kernel/syscall.c`, `kernel/syscall.h`, `kernel/sysproc.c`
- User-space stubs: `user/usys.pl`, `user/user.h`
- Supplied files in the Scheduling zip: per-section `testloop.c`, `syspriority.c`, and (C2) `sectionc.patch`
