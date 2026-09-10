# MLFQ Scheduler with Lottery Scheduling and Aging — Full Explanation

## What is Scheduling?

Scheduling is how an operating system decides which process gets to use the CPU and for how long. The default xv6 scheduler is a plain **round-robin**: it runs every process for one tick, then moves to the next, giving every process equal CPU time regardless of how it behaves (CPU-bound vs I/O-bound).

This assignment replaces that default scheduler with a **two-level Multilevel Feedback Queue (MLFQ)** that uses **lottery scheduling** in the top queue and **round-robin** in the bottom queue, with an **aging mechanism** to prevent starvation.

---

## What Does the Assignment Want?

1. **Two Queues**: Queue 0 (high priority, lottery scheduling) and Queue 1 (low priority, round-robin).
2. **Queue Transitions**: Processes move between queues based on behavior:
   - Exhausting a time slice **demotes** a process to Queue 1.
   - Voluntarily yielding (e.g., calling `sleep()`) **promotes** a process to Queue 0.
3. **Lottery Scheduling in Queue 0**: Processes compete for CPU time proportional to their ticket count. No floating-point math allowed.
4. **Aging**: A process waiting too long in Queue 1 (more than `WAIT_THRESH=6` ticks) gets promoted to Queue 0 to prevent starvation.
5. **Two New System Calls**: `settickets(int)` to set a process's ticket count, and `getpinfo(struct pstat *)` to query scheduling statistics.
6. **Fork Inheritance**: Child processes inherit the parent's ticket count.
7. **Logging**: Colored log messages for DEMO, PROMO, BOOST, and LOTTERY events.
8. **Two Test Programs**: `dummyproc` and `testprocinfo` to exercise and observe the scheduler.

---

## Part 1: Core Definitions and Header Files

### What We Changed

#### `kernel/param.h` — Added 4 Constants

```c
#define TIME_LIMIT_0     2   // Queue 0 time slice (ticks)
#define TIME_LIMIT_1     4   // Queue 1 time slice (ticks)
#define WAIT_THRESH      6   // Aging threshold (ticks)
#define DEFAULT_TICKETS  10  // Default tickets for new processes
```

**Why**: These constants define the scheduler's behavior. `TIME_LIMIT_0=2` means a process in Queue 0 gets at most 2 ticks before being demoted. `TIME_LIMIT_1=4` means Queue 1 processes get 4 ticks. `WAIT_THRESH=6` means a process waiting more than 6 ticks in Queue 1 gets promoted. `DEFAULT_TICKETS=10` is the starting ticket count for every new process.

**Consequence**: Every part of the scheduler references these values. Changing them changes the entire scheduling behavior.

#### `kernel/pstat.h` — New File

Created a new header defining `struct pstat`, which holds per-process scheduling statistics: PID, in-use flag, queue number, waiting time, running time, times scheduled, original and current ticket counts, and queue ticks for both queues.

**Why**: This is the exact struct the spec requires. The `getpinfo` system call fills this struct and copies it to user space. It must not be modified.

**Consequence**: User-space programs like `testprocinfo` can call `getpinfo()` to get a snapshot of every process's scheduling state.

#### `kernel/proc.h` — Extended `struct proc`

Added 8 fields to the per-process struct:

```c
int inQ;                     // Current queue (0 or 1)
int tickets_original;        // Base ticket count
int tickets_current;         // Remaining tickets in current lottery cycle
int ticks_consumed;          // Ticks consumed in current time slice
int waiting_time;            // Ticks spent waiting in RUNNABLE state
int running_time;            // Times scheduled before time slice expired
int times_scheduled;         // Total times scheduled
uint queue_ticks[2];         // Total ticks in queue 0 and queue 1
```

Also added `extern struct proc proc[];` so other files (trap.c, syscall.c) can access the process table.

**Why**: The scheduler needs to track per-process MLFQ state. Without these fields, it's impossible to know which queue a process is in, how many tickets it has, or how long it has been waiting.

**Consequence**: Every new process starts with these fields initialized (in `allocproc()`). The scheduler reads and updates them on every scheduling decision.

---

## Part 2: System Calls and Fork Inheritance

### What We Changed

#### `kernel/syscall.h` — Added System Call Numbers

```c
#define SYS_settickets 22
#define SYS_getpinfo   23
```

**Why**: xv6 dispatches system calls by number. Each new syscall needs a unique number in this file.

#### `kernel/syscall.c` — Implemented Both System Calls

**`sys_settickets`**: Reads the integer argument via `argint()`. If the number is less than 1, resets both `tickets_original` and `tickets_current` to `DEFAULT_TICKETS` and returns -1. Otherwise, sets both to the given number and returns 0.

**Why**: This lets user processes request a different share of CPU time. A process calling `settickets(30)` gets 3x the CPU share of one with 10 tickets.

**`sys_getpinfo`**: Reads a user pointer via `argaddr()`. Iterates over all 64 process slots, acquires each process's lock, copies its stats into a local `struct pstat`, then uses `copyout()` to transfer the entire struct to user space.

**Why**: This gives user-space programs a snapshot of the scheduler's state. Without it, you can't observe what the scheduler is doing.

Added `#include "pstat.h"` at the top so the struct is available.

#### `kernel/proc.c` — `allocproc()` and `fork()`

In `allocproc()`, after setting up the kernel context, we initialize all MLFQ fields:

```c
p->inQ = 0;
p->tickets_original = DEFAULT_TICKETS;
p->tickets_current = DEFAULT_TICKETS;
p->ticks_consumed = 0;
p->waiting_time = 0;
p->running_time = 0;
p->times_scheduled = 0;
p->queue_ticks[0] = 0;
p->queue_ticks[1] = 0;
```

**Why**: Without this, new processes would have garbage values in their scheduling fields, causing undefined behavior in the scheduler.

In `fork()`, after copying the process name, we add:

```c
np->tickets_original = p->tickets_original;
np->tickets_current = p->tickets_original;
```

**Why**: The spec requires child processes to inherit the parent's ticket count. If a parent has 30 tickets when it forks, the child should also start with 30.

#### `user/user.h` — Added Prototypes

```c
int settickets(int);
int getpinfo(void*);
```

**Why**: User programs need function declarations to call these syscalls.

#### `user/usys.pl` — Added Stubs

```perl
entry("settickets");
entry("getpinfo");
```

**Why**: This script generates the assembly stubs that bridge user space to kernel space via `ecall`. Without these stubs, calling the syscalls from C would crash.

---

## Part 3: Scheduler Logic and Test Programs

### What We Changed

#### `kernel/proc.c` — The Scheduler

**PRNG (Pseudo-Random Number Generator)**: Added a deterministic linear congruential generator (LCG) at the top of the file:

```c
static unsigned long rand_seed = 123456789;
static int rand(int max) {
  if(max <= 0) return 0;
  rand_seed = rand_seed * 1103515245 + 12345;
  return (int)((rand_seed / 65536) % 32768) % max;
}
```

**Why**: Lottery scheduling needs a random number to pick a winner. The spec requires a deterministic seed so runs are reproducible.

**Global toggle**: `int print_logs = 1;` controls whether DEMO/PROMO/BOOST/LOTTERY messages are printed.

**`scheduler()` — Complete Rewrite**: The old scheduler was a simple round-robin loop. The new one has 3 steps:

1. **Aging sweep**: Loop through all RUNNABLE processes in Queue 1. If `waiting_time > WAIT_THRESH`, promote to Queue 0, reset counters, print BOOST.

2. **Queue 0 (Lottery)**: Sum `tickets_current` of all RUNNABLE Queue 0 processes. If all are 0, reset them back to `tickets_original`. Draw a random number `r` in `[0, total_tickets)`. Walk processes accumulating tickets until the running sum exceeds `r`. That process wins. Decrement its `tickets_current` by 1, print LOTTERY, context-switch to it.

3. **Queue 1 (Round-Robin)**: If Queue 0 is empty, pick the first RUNNABLE process in Queue 1. Context-switch to it.

After each context switch, the scheduler restarts from the top (Queue 0 search).

**Why**: This implements the MLFQ behavior: top-down priority search, lottery in Q0, round-robin in Q1, aging to prevent starvation.

**`yield()` — Complete Rewrite**: When a process gives up the CPU:

- Increments `ticks_consumed` and `queue_ticks[inQ]`.
- **Queue 0 + time exhausted** (`ticks_consumed >= TIME_LIMIT_0`): Demote to Queue 1, increment `running_time`, print DEMO, reset `ticks_consumed`.
- **Queue 0 + voluntary yield** (before time exhausted): Stay in Queue 0, print PROMO, reset `ticks_consumed`.
- **Queue 1 + time exhausted**: Stay in Queue 1, increment `running_time`, reset `ticks_consumed`.
- **Queue 1 + voluntary yield**: Promote to Queue 0, print PROMO, reset `ticks_consumed`.

**Why**: This handles the core MLFQ queue transition logic. Demotion punishes CPU-bound processes. Promotion rewards I/O-bound processes.

#### `kernel/trap.c` — Timer Interrupt

In `clockintr()`, added:

```c
struct proc *p;
for(p = proc; p < &proc[NPROC]; p++) {
  acquire(&p->lock);
  if(p->state == RUNNABLE) {
    p->waiting_time++;
  }
  release(&p->lock);
}
```

**Why**: Aging needs to know how long each process has been waiting. This increments `waiting_time` for every RUNNABLE process on every tick. When `waiting_time` exceeds `WAIT_THRESH`, the scheduler promotes the process.

#### `user/testprocinfo.c` — Test Program

Calls `getpinfo(&st)` and prints a table of all active processes with: PID, In Use, In Q, Waiting time, Running time, Times scheduled, Original tickets, Current tickets, q0 ticks, q1 ticks. Skips processes with PID 0.

**Why**: This is the observation tool. It shows you what the scheduler is doing with every process.

#### `user/dummyproc.c` — Test Program

Takes `<tickets> <iterations>` arguments. Calls `settickets()`. Forks a child:
- **Parent**: CPU-bound tight loop (triggers DEMO — exhausts time slice).
- **Child**: Loop with periodic `sleep(1)` (triggers PROMO — voluntary yield).

**Why**: The parent demonstrates demotion. The child demonstrates promotion. Together they exercise the full MLFQ transition logic.

#### `Makefile`

- Changed `CPUS := 3` to `CPUS := 1` (single core per spec).
- Added `$U/_dummyproc` and `$U/_testprocinfo` to UPROGS.

**Why**: The spec requires single-core. The new programs need to be compiled and linked into the filesystem image.

---

## How to Run

```bash
cd edit/xv6-riscv
make clean && make qemu
```

You'll see the xv6 shell prompt `$`.

## How to Test

### Test 1: Basic System Call Check

```
$ testprocinfo
```

**Expected**: A table showing `init` (pid 1) and `sh` (pid 2) with 10 tickets each, all in queue 0.

### Test 2: Lottery Scheduling

```
$ dummyproc 30 1000000000 &
$ dummyproc 10 1000000000 &
$ testprocinfo
```

**Expected**: The 30-ticket process has ~3x more `Times sched` and `q0 ticks` than the 10-ticket one.

### Test 3: Demotion and Promotion (with logging enabled)

```
$ dummyproc 20 500000000 &
```

**Expected logs** (with `print_logs = 1`):
- Parent: `DEMO: Process <pid> (dummyproc) ran for 2 time ticks, demoted to queue 1`
- Child: `PROMO: Process <pid> (dummyproc) ran for 1 time ticks, promoted to queue 0`

### Test 4: Aging (BOOST)

```
$ dummyproc 10 2000000000 &
```

**Expected log**: After a process waits >6 ticks in Queue 1:
`BOOST: Process <pid> (dummyproc) waited for 6 ticks, promoted to queue 0`

### Test 5: Ticket Reset

Run `testprocinfo` and observe that `Curr tix` decreases over time as processes win lotteries. When all reach 0, they reset back to `Orig tix`.

---

## Summary of All Files Changed

| File | Lines Changed | Purpose |
|------|--------------|---------|
| `kernel/param.h` | +6 | Scheduler constants |
| `kernel/pstat.h` | +18 (new) | Process stats struct |
| `kernel/proc.h` | +12 | MLFQ fields in struct proc |
| `kernel/proc.c` | +162 | Scheduler, yield, PRNG, alloc, fork |
| `kernel/syscall.h` | +2 | Syscall numbers |
| `kernel/syscall.c` | +55 | settickets, getpinfo implementations |
| `kernel/trap.c` | +10 | Waiting time tracking |
| `kernel/defs.h` | +1 | Forward declaration |
| `user/user.h` | +2 | Syscall prototypes |
| `user/usys.pl` | +2 | Syscall stubs |
| `user/dummyproc.c` | +46 (new) | Test program |
| `user/testprocinfo.c` | +37 (new) | Test program |
| `Makefile` | +3/-2 | CPUS=1, UPROGS |

**Total**: 13 files changed, ~350 lines of new code.
