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
np->tickets_current = p->tickets_current;
```

**Why**: The spec says "A child process must inherit its parent's current ticket count." The child gets the parent's remaining `tickets_current` (not `tickets_original`), so a parent that has already burned some tickets forks a child with the same depleted count — no free ticket regeneration.

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

**`scheduler()` — Complete Rewrite**: The old scheduler was a simple round-robin loop. The new one is protected by `mlfq_lock` for multi-CPU safety and has 3 steps:

1. **Aging sweep** (under `mlfq_lock`): Loop through all RUNNABLE processes in Queue 1. If `waiting_time > WAIT_THRESH`, promote to Queue 0, reset counters, print BOOST.

2. **Queue 0 (Lottery)** (under `mlfq_lock`): Sum `tickets_current` of all RUNNABLE Queue 0 processes. If all are 0, reset them back to `tickets_original`. Draw a random number `r` in `[0, total_tickets)`. Walk processes accumulating tickets until the running sum exceeds `r`. That process wins. Decrement its `tickets_current` by 1, set `state = RUNNING`, print LOTTERY. **Release `mlfq_lock` before `swtch()`** so other CPUs can schedule in parallel.

3. **Queue 1 (Round-Robin)** (under `mlfq_lock`): If Queue 0 is empty, pick the first RUNNABLE process in Queue 1. Set `state = RUNNING`. Release `mlfq_lock` before `swtch()`.

4. **Resume loop** (after `mlfq_lock` release): After `swtch()` returns, if the process is still RUNNABLE, still in Queue 0, and hasn't exhausted `ticks_consumed`, resume it directly instead of drawing a new lottery winner. This ensures a process runs its full 2-tick time slice without losing the CPU to a re-draw.

**Why**: The `mlfq_lock` serializes the selection decision so two CPUs can't both pick the same process or both reset tickets. Releasing it before `swtch()` allows parallel execution. The resume loop implements the spec's requirement that "queue 0's time slice allows a process to run several ticks in a row."

**`yield()` — Timer-driven preemption only**: In xv6, `yield()` is called on *every timer tick* from `usertrap()`. It is **not** called on voluntary yields (those go through `sleep()`). So `yield()` only does:

- Increments `ticks_consumed` and `queue_ticks[inQ]`.
- **Queue 0 + time exhausted** (`ticks_consumed >= TIME_LIMIT_0`): Demote to Queue 1, increment `running_time`, print DEMO, reset `ticks_consumed`.
- **Queue 1 + time exhausted** (`ticks_consumed >= TIME_LIMIT_1`): Stay in Queue 1, increment `running_time`, reset `ticks_consumed`.
- **Otherwise**: Do nothing — just mark RUNNABLE and return. The scheduler's resume loop will re-run this process.

**Critical insight**: `yield()` never promotes. Promotion happens in `sleep()` (see below).

**`sleep()` — Voluntary yield promotion**: When a process voluntarily blocks (I/O, pipe, `sleep()` syscall), it calls `sleep()` which sets `state = SLEEPING` and calls `sched()` directly — never going through `yield()`. Before sleeping:

- If the process is in Queue 1 and hasn't used its full time slice (`ticks_consumed < TIME_LIMIT_1`), promote it to Queue 0 and print PROMO.
- Reset `ticks_consumed` to 0 since the process is giving up the CPU.

**Why**: This correctly separates timer-driven demotion (in `yield()`) from voluntary promotion (in `sleep()`). A process that sleeps before exhausting its time slice is I/O-bound and deserves higher priority.

#### `kernel/trap.c` — Timer Interrupt

In `clockintr()`, added (guarded by `cpuid() == 0`):

```c
if(cpuid() == 0) {
  struct proc *p;
  for(p = proc; p < &proc[NPROC]; p++) {
    acquire(&p->lock);
    if(p->state == RUNNABLE) {
      p->waiting_time++;
    }
    release(&p->lock);
  }
}
```

**Why**: Aging needs to know how long each process has been waiting. This increments `waiting_time` for every RUNNABLE process on every tick. The `cpuid() == 0` guard ensures only one CPU does this — with 3 CPUs, without the guard, `waiting_time` would grow 3x too fast and processes would be promoted after ~2 real ticks instead of 6.

#### `user/testprocinfo.c` — Test Program

Calls `getpinfo(&st)` and prints a table of all active processes with: PID, In Use, In Q, Waiting time, Running time, Times scheduled, Original tickets, Current tickets, q0 ticks, q1 ticks. Skips processes with PID 0.

**Why**: This is the observation tool. It shows you what the scheduler is doing with every process.

#### `user/dummyproc.c` — Test Program

Takes `<tickets> <iterations>` arguments. Calls `settickets()`. Forks a child:
- **Parent**: CPU-bound LCG loop (`sink = sink * 1103515245u + i; sink ^= sink >> 13`) — non-optimizable work that actually stalls the CPU, triggering DEMO after 2 ticks.
- **Child**: Loop with periodic `sleep(1)` (triggers PROMO — voluntary yield).

**Why**: The LCG body can't be optimized away by the compiler (unlike `volatile int x = i * i` which finishes in nanoseconds). With ≥ 10^9 iterations, the parent runs long enough to burn 2 timer ticks (~200ms on QEMU), which is the minimum needed for DEMO to fire.

#### `Makefile`

- Set `CPUS := 3` for multi-CPU bonus.
- Added `-march=rv64gc` to CFLAGS and `-cpu max` to QEMUOPTS for QEMU 10.2.1 compatibility.
- Added `$U/_dummyproc`, `$U/_testprocinfo`, and `$U/_usertests` to UPROGS.

**Why**: The multi-CPU bonus requires `CPUS > 1`. The architecture flag prevents QEMU silent hangs. The test programs need to be compiled into the filesystem image.

---

## Part 4: Multi-CPU Bonus

### The Problem

With `CPUS > 1`, three things break:
1. **Double-pick**: Two CPUs can both read stale ticket totals and pick the same process.
2. **Double-reset**: Two CPUs can both see `total_tickets == 0` and both reset tickets.
3. **Fast aging**: Every CPU's timer IRQ increments `waiting_time`, so aging fires NCPUS× too fast.

### The Fix: One Global Lock

Added `struct spinlock mlfq_lock` in `kernel/proc.c` and initialized it in `procinit()`. The entire selection block (aging sweep + lottery + RR fallback) runs under `mlfq_lock`. The lock is released **before** `swtch()` so other CPUs can schedule in parallel.

```
acquire(&mlfq_lock);
  // aging sweep, lottery, RR fallback — all under lock
  chosen_p->state = RUNNING;  // prevents double-pick
release(&mlfq_lock);
swtch(...);  // parallel execution
```

For `waiting_time++` in `clockintr()`, added `if(cpuid() == 0)` so only one CPU increments the aging clock.

**Why this works**: Selection is atomic (no double-pick or double-reset). Execution is parallel (lock released before context switch). The `cpuid()==0` guard ensures aging runs at the correct rate.

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
$ dummyproc 43 1000000000 &
$ dummyproc 5 1500000000 &
$ testprocinfo
```

**Expected**: The 43-ticket process gets scheduled ~8x more than the 5-ticket one. `Curr tix` decreases as lotteries are won.

### Test 3: Demotion and Promotion (DEMO + PROMO)

```
$ dummyproc 20 1000000000 &
```

**Expected logs** (with `print_logs = 1`):
- Parent: `DEMO: Process <pid> (dummyproc) ran for 2 time ticks, demoted to queue 1`
- Child: `PROMO: Process <pid> (dummyproc) ran for N time ticks, promoted to queue 0`

The parent's LCG loop runs long enough to burn 2 ticks, triggering DEMO. The child sleeps periodically, triggering PROMO.

### Test 4: Aging (BOOST)

```
$ dummyproc 10 2000000000 &
```

**Expected log**: After a process waits >6 ticks in Queue 1 while others hog Queue 0:
`BOOST: Process <pid> (dummyproc) waited for 6 ticks, promoted to queue 0`

### Test 5: Multi-CPU (Bonus)

```
$ dummyproc 43 1000000000 &
$ dummyproc 5 1500000000 &
$ dummyproc 20 1500000000 &
$ testprocinfo
```

**Expected**: Multiple LOTTERY lines interleaved (one per CPU), DEMO firing for different PIDs on different CPUs, no duplicate state = RUNNING for the same PID.

### Test 6: Ticket Reset

Run `testprocinfo` and observe that `Curr tix` decreases over time as processes win lotteries. When all reach 0, they reset back to `Orig tix`.

---

## Summary of All Files Changed

| File | Lines Changed | Purpose |
|------|--------------|---------|
| `kernel/param.h` | +6 | Scheduler constants |
| `kernel/pstat.h` | +18 (new) | Process stats struct |
| `kernel/proc.h` | +12 | MLFQ fields in struct proc |
| `kernel/proc.c` | +190 | Scheduler (mlfq_lock), yield, sleep PROMO, PRNG, alloc, fork |
| `kernel/syscall.h` | +2 | Syscall numbers |
| `kernel/syscall.c` | +55 | settickets, getpinfo implementations |
| `kernel/trap.c` | +12 | Waiting time tracking (cpuid==0 guard) |
| `kernel/defs.h` | +1 | Forward declaration |
| `user/user.h` | +2 | Syscall prototypes |
| `user/usys.pl` | +2 | Syscall stubs |
| `user/dummyproc.c` | +46 (new) | Test program (LCG parent loop) |
| `user/testprocinfo.c` | +37 (new) | Test program |
| `user/usertests.c` | +1 | Fix rwsbrk signature |
| `Makefile` | +5/-3 | CPUS=3, -march=rv64gc, -cpu max, UPROGS |

**Total**: 14 files changed, ~400 lines of new code.
