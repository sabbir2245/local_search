Conversation with Gemini

How to start

To successfully implement the xv6 Multilevel Feedback Queue (MLFQ) with Lottery Scheduling and Aging assignment, the best approach is to break it down into incremental, testable phases.  
PDF

Below is a step-by-step roadmap to guide your implementation.
Step 1: Core Definitions and Header Files

Before modifying scheduler logic, add the required configuration constants and system call definitions.  
PDF

    kernel/param.h: Define the 4 specification parameters:  
    PDF
    C

    #define TIME_LIMIT_0     2
    #define TIME_LIMIT_1     4
    #define WAIT_THRESH      6
    #define DEFAULT_TICKETS  10

    kernel/pstat.h: Create this file and add the exact struct pstat definition given in the specification.  
    PDF

    kernel/proc.h: Extend struct proc to track MLFQ and statistic metrics:  
    PDF

        int inQ (0 or 1)  
        PDF

        int tickets_original and int tickets_current

          
        PDF

        int ticks_consumed (ticks spent in current queue turn)  
        PDF

        int waiting_time (ticks spent in RUNNABLE state without being scheduled)  
        PDF

        int running_time

          
        PDF

        int times_scheduled

          
        PDF

        uint queue_ticks[2]

          
        PDF

Step 2: System Calls & Fork Inheritance

Set up settickets and getpinfo, and ensure ticket counts copy properly across process creation.  
PDF

    System Call Infrastructure:

        Add entry numbers for SYS_settickets and SYS_getpinfo in kernel/syscall.h.

        Add prototypes in user/user.h and system call declarations in kernel/syscall.c.

        Implement stubs in user/usys.pl.

    sys_settickets:

        Read the integer argument using argint().  
        PDF

        If number < 1, set tickets_original (and tickets_current) to DEFAULT_TICKETS and return -1.  
        PDF

        Otherwise, set both to number and return 0.  
        PDF

    sys_getpinfo:

        Read user pointer using argaddr().  
        PDF

        Iterate over proc[], gather process details into a local struct pstat, and use copyout() to transfer data safely to user space.  
        PDF

    fork() in kernel/proc.c:

        Inherit tickets: np->tickets_original = p->tickets_original; np->tickets_current = p->tickets_original;

          
        PDF

        Initialize child process fields (inQ = 0, counters to 0).  
        PDF

Step 3: Random Number Generator

You need a kernel-space deterministic Pseudo-Random Number Generator (PRNG) for lottery draws.  
PDF

    Add a linear congruential generator (LCG) or xorshift PRNG inside kernel/proc.c:  
    PDF
    C

    static unsigned long seed = 123456789;
    int random(int max) { // returns [0, max)
        seed = seed * 1103515245 + 12345;
        return (int)((seed / 65536) % 32768) % max;
    }

Step 4: MLFQ Scheduler Logic (kernel/proc.c)

Modify scheduler() in kernel/proc.c to implement top-down selection, queue 0 lottery, queue 1 round-robin, and aging.  
PDF

                     +-----------------------+
                     |  New Process Enters   |
                     +-----------+-----------+
                                 |
                                 v
                       +-------------------+
                       |      Queue 0      |
                       |(Lottery, Limit=2) |
                       +---------+---------+
                                 |
                  +--------------+--------------+
                  |                             |
      Time Slice Exhausted              Voluntary Yield
                  |                             |
                  v                             v
       +-------------------+          +-------------------+
       |      Queue 1      |          |      Queue 0      |
       | (RR, Limit = 4)   |          +-------------------+
       +---------+---------+
                 |
      +----------+----------+
      |                     |
Voluntary Yield          Aging (> 6 ticks)
      |                     |
      v                     v
+-----------+         +-----------+
|  Queue 0  |         |  Queue 0  |
+-----------+         +-----------+

    Aging Sweep:

        Loop through all RUNNABLE processes in Queue 1.  
        PDF

        If waiting_time > WAIT_THRESH, promote to Queue 0 (inQ = 0), reset waiting_time = 0, and log BOOST.  
        PDF

    Queue 0 Search (Lottery):

        Calculate sum N of tickets_current for all RUNNABLE processes in Queue 0.  
        PDF

        If all active Queue 0 processes have tickets_current == 0, reset all Queue 0 processes back to tickets_original and recalculate N.  
        PDF

        Draw r∈[0,N).  
        PDF

        Walk Queue 0 processes, accumulating tickets until running sum >r.  
        PDF

        Decrement the winner's tickets_current by 1 and log LOTTERY.  
        PDF

    Queue 1 Search (Round-Robin):

        If Queue 0 is empty, select the next RUNNABLE process in Queue 1 sequentially.  
        PDF

    Context Switch & Execution:

        Update statistics (times_scheduled++, track queue_ticks).  
        PDF

        Context switch to process.  
        PDF

Step 5: Handling Yields, Demotions, and Promotions

Timer interrupts force context switches back to scheduler() via yield(). Modify trap/yield handling or yield() in kernel/proc.c:  
PDF

    Tick Accounting:

        Increment ticks_consumed on each timer tick for the running process.  
        PDF

        Increment waiting_time for all other RUNNABLE processes sitting in queues.  
        PDF

    Time Slice Exhaustion (Demotion):

        If in Queue 0 and ticks_consumed >= TIME_LIMIT_0: demote to Queue 1 (inQ = 1), reset ticks_consumed = 0, and log DEMO.  
        PDF

        If in Queue 1 and ticks_consumed >= TIME_LIMIT_1: remain in Queue 1, reset ticks_consumed = 0, and log DEMO.  
        PDF

    Voluntary Yield (Promotion):

        If a process yields voluntarily (e.g., via sleep() for I/O) before using its full time slice: promote to Queue 0 (inQ = 0), reset ticks_consumed = 0, and log PROMO.  
        PDF

Step 6: User Programs for Testing

    user/testprocinfo.c:

        Call getpinfo(&st) and display process metrics (PID, state, queue, wait/run times, ticket states) in tabular format.  
        PDF

        Ensure PID 0 is ignored.  
        PDF

    user/dummyproc.c:

        Accepts arguments <tickets> <iterations>.  
        PDF

        Calls settickets().  
        PDF

        Forks a child process that periodically sleeps inside its loop to test voluntary yield promotion (PROMO).  
        PDF

        Run long computation loops in parent and child so testprocinfo can observe them.  
        PDF

    Update UPROGS in Makefile to include _dummyproc and _testprocinfo.  
    PDF

Suggested Order of Execution
1
Setup Headers & Parameters
Step 1

Define macros in param.h, create pstat.h, and update struct proc in proc.h.
2
Add System Calls
Step 2

Implement settickets and getpinfo along with fork ticket inheritance.
3
Build User Test Programs
Step 3

Implement testprocinfo.c and basic dummyproc.c to test system calls before changing scheduler behavior.
4
Implement Scheduler & Logging
Step 4

Rewrite scheduler() in proc.c with top-down MLFQ, Queue 0 lottery, Queue 1 RR, and aging.
5
Verification
Step 5

Run dummyproc in background and check testprocinfo output to verify demotions, promotions, and ticket share ratios.

Can you show me the exact code changes needed for param.h, pstat.h, and struct proc in proc.h?

Here are the exact modifications for kernel/param.h, kernel/pstat.h, and kernel/proc.h.
1. Update kernel/param.h

Add the four required specification constants at the end of kernel/param.h:  
PDF
C

// MLFQ and Lottery Scheduling Parameters
#define TIME_LIMIT_0     2   // Time slice (ticks) for Queue 0
#define TIME_LIMIT_1     4   // Time slice (ticks) for Queue 1
#define WAIT_THRESH      6   // Ticks waiting in Q1 before promotion (aging)
#define DEFAULT_TICKETS  10  // Default tickets assigned to new processes

2. Create kernel/pstat.h

Create a new file at kernel/pstat.h with the exact structure required by the specification:  
PDF
C

#ifndef _PSTAT_H_
#define _PSTAT_H_

#include "param.h"

struct pstat {
  int pid[NPROC];              // process ID of each process
  int inuse[NPROC];            // whether this proc slot is in use (1 or 0)
  int inQ[NPROC];              // current queue of the process
  int waiting_time[NPROC];     // ticks spent waiting before being scheduled
  int running_time[NPROC];     // times scheduled before time slice was used
  int times_scheduled[NPROC];  // total times this process was scheduled
  int tickets_original[NPROC]; // tickets originally assigned
  int tickets_current[NPROC];  // tickets currently held
  uint queue_ticks[NPROC][2];  // total ticks spent in each queue
};

#endif // _PSTAT_H_

3. Update kernel/proc.h

Add the MLFQ state tracking fields to struct proc inside kernel/proc.h:  
PDF
C

// Per-process state
struct proc {
  struct spinlock lock;

  // p->lock must be held when using these:
  enum procstate state;        // Process state
  void *chan;                  // If non-zero, sleeping on chan
  int killed;                  // If non-zero, have been killed
  int xstate;                  // Exit status to be returned to parent's wait
  int pid;                     // Process ID

  // wait_lock must be held when using this:
  struct proc *parent;         // Parent process

  // these are private to the process, so p->lock need not be held.
  uint64 kstack;               // Virtual address of kernel stack
  uint64 sz;                   // Size of process memory (bytes)
  pagetable_t pagetable;       // User page table
  struct trapframe *trapframe; // data page for trampoline.S
  struct context context;      // swtch() here to run process
  struct file *ofile[NOFILE];  // Open files
  struct inode *cwd;           // Current directory
  char name[16];               // Process name (debugging)

  // MLFQ Scheduling Fields
  int inQ;                     // Current queue index (0 = Lottery, 1 = Round Robin)
  int tickets_original;        // Base ticket count
  int tickets_current;         // Remaining tickets in current lottery cycle
  int ticks_consumed;          // Ticks consumed in current time slice
  int waiting_time;            // Ticks spent waiting in RUNNABLE state
  int running_time;            // Times scheduled before time slice expired
  int times_scheduled;         // Total times scheduled
  uint queue_ticks[2];         // Total ticks accumulated in queue 0 and queue 1
};

Next Steps

When initializing processes inside allocproc() in kernel/proc.c, initialize these new fields:  
PDF

    p->inQ = 0;

      
    PDF

    p->tickets_original = DEFAULT_TICKETS;

      
    PDF

    p->tickets_current = DEFAULT_TICKETS;

      
    PDF

    p->ticks_consumed = 0;

      
    PDF

    p->waiting_time = 0;

      
    PDF

    p->running_time = 0;

      
    PDF

    p->times_scheduled = 0;

      
    PDF

    p->queue_ticks[0] = 0;

      
    PDF

    p->queue_ticks[1] = 0;

      
    PDF

Want me to draft the complete scheduler loop for kernel/proc.c?
Yes

Can you provide a code draft for the MLFQ scheduler loop in kernel/proc.c?

Here is a complete draft for the MLFQ scheduler loop and its related helper functions in kernel/proc.c.  
PDF
Pseudo-Random Number Generator (Kernel-Level)

Add a deterministic kernel-level Pseudo-Random Number Generator (PRNG) at the top of kernel/proc.c to handle lottery draws:  
PDF
C

// Deterministic linear congruential generator (LCG) for kernel lottery draws
static unsigned long rand_seed = 123456789;

static int
rand(int max)
{
  if (max <= 0)
    return 0;
  rand_seed = rand_seed * 1103515245 + 12345;
  return (int)((rand_seed / 65536) % 32768) % max;
}

MLFQ scheduler() Loop

Replace the existing scheduler() function in kernel/proc.c with this implementation:  
PDF
C

void
scheduler(void)
{
  struct proc *p;
  struct cpu *c = mycpu();
  c->proc = 0;

  for(;;){
    // Avoid deadlock on CPU pause
    intr_on();

    // ------------------------------------------------------------------------
    // STEP 1: Aging Mechanism (Queue 1 -> Queue 0)
    // ------------------------------------------------------------------------
    for(p = proc; p < &proc[NPROC]; p++) {
      acquire(&p->lock);
      if(p->state == RUNNABLE && p->inQ == 1 && p->waiting_time > WAIT_THRESH) {
        p->inQ = 0;
        p->ticks_consumed = 0;
        p->waiting_time = 0;
        if(print_logs) {
          printf("\033[0;33mBOOST: Process %d (%s) waited for %d ticks, promoted to queue 0\033[0m\n",
                 p->pid, p->name, WAIT_THRESH);
        }
      }
      release(&p->lock);
    }

    // ------------------------------------------------------------------------
    // STEP 2: Check Queue 0 (Lottery Scheduling)
    // ------------------------------------------------------------------------
    int total_tickets = 0;
    for(p = proc; p < &proc[NPROC]; p++) {
      acquire(&p->lock);
      if(p->state == RUNNABLE && p->inQ == 0) {
        total_tickets += p->tickets_current;
      }
      release(&p->lock);
    }

    // Reset current tickets if all runnable Queue 0 processes hit 0
    if(total_tickets == 0) {
      for(p = proc; p < &proc[NPROC]; p++) {
        acquire(&p->lock);
        if(p->state == RUNNABLE && p->inQ == 0) {
          p->tickets_current = p->tickets_original;
          total_tickets += p->tickets_current;
        }
        release(&p->lock);
      }
    }

    // If there are runnable processes in Queue 0, hold the lottery
    if(total_tickets > 0) {
      int winner_ticket = rand(total_tickets); // Range [0, total_tickets)
      int ticket_sum = 0;
      struct proc *chosen_p = 0;

      for(p = proc; p < &proc[NPROC]; p++) {
        acquire(&p->lock);
        if(p->state == RUNNABLE && p->inQ == 0) {
          ticket_sum += p->tickets_current;
          if(ticket_sum > winner_ticket && chosen_p == 0) {
            chosen_p = p;
            // Hold lock for the scheduled process
            continue;
          }
        }
        release(&p->lock);
      }

      if(chosen_p != 0) {
        // Lottery winner execution
        chosen_p->tickets_current--;
        chosen_p->times_scheduled++;
        chosen_p->waiting_time = 0;

        if(print_logs) {
          printf("\033[0;32mLOTTERY: Process %d (%s) won in queue 0 with tickets %d\033[0m\n",
                 chosen_p->pid, chosen_p->name, chosen_p->tickets_current);
        }

        chosen_p->state = RUNNING;
        c->proc = chosen_p;
        swtch(&c->context, &chosen_p->context);

        // Process yields back to scheduler
        c->proc = 0;
        release(&chosen_p->lock);
        continue; // Restart top-down search from Queue 0
      }
    }

    // ------------------------------------------------------------------------
    // STEP 3: Check Queue 1 (Round-Robin)
    // ------------------------------------------------------------------------
    for(p = proc; p < &proc[NPROC]; p++) {
      acquire(&p->lock);
      if(p->state == RUNNABLE && p->inQ == 1) {
        p->times_scheduled++;
        p->waiting_time = 0;

        p->state = RUNNING;
        c->proc = p;
        swtch(&c->context, &p->context);

        // Process yields back to scheduler
        c->proc = 0;
        release(&p->lock);

        // Restart top-down search from Queue 0
        break;
      }
      release(&p->lock);
    }
  }
}

Update yield() for Queue Transitions and Logging

When a process gives up the CPU (timer tick or voluntary yield), update yield() in kernel/proc.c to manage demotion/promotion:  
PDF
C

void
yield(void)
{
  struct proc *p = myproc();
  acquire(&p->lock);

  p->ticks_consumed++;
  p->queue_ticks[p->inQ]++;

  if(p->inQ == 0) {
    if(p->ticks_consumed >= TIME_LIMIT_0) {
      // Used full time slice -> Demote to Queue 1
      p->inQ = 1;
      p->running_time++;
      if(print_logs) {
        printf("\033[0;31mDEMO: Process %d (%s) ran for %d time ticks, demoted to queue 1\033[0m\n",
               p->pid, p->name, p->ticks_consumed);
      }
      p->ticks_consumed = 0;
    } else {
      // Voluntary yield (e.g., sleep/IO) before time slice finished -> Promote
      p->inQ = 0;
      if(print_logs) {
        printf("\033[0;34mPROMO: Process %d (%s) ran for %d time ticks, promoted to queue 0\033[0m\n",
               p->pid, p->name, p->ticks_consumed);
      }
      p->ticks_consumed = 0;
    }
  } else if(p->inQ == 1) {
    if(p->ticks_consumed >= TIME_LIMIT_1) {
      // Exhausted Q1 time slice -> Remain in Queue 1
      p->running_time++;
      p->ticks_consumed = 0;
    } else {
      // Voluntary yield in Q1 -> Promote to Queue 0
      p->inQ = 0;
      if(print_logs) {
        printf("\033[0;34mPROMO: Process %d (%s) ran for %d time ticks, promoted to queue 0\033[0m\n",
               p->pid, p->name, p->ticks_consumed);
      }
      p->ticks_consumed = 0;
    }
  }

  p->state = RUNNABLE;
  sched();
  release(&p->lock);
}

Timer Interrupt Updates (kernel/trap.c)

In clockintr() inside kernel/trap.c, increment waiting_time for all RUNNABLE processes every tick so aging can track wait times:  
PDF
C

// Inside clockintr() in trap.c
for(struct proc *p = proc; p < &proc[NPROC]; p++) {
  if(p->state == RUNNABLE) {
    p->waiting_time++;
  }
}

Want me to draft testprocinfo.c and dummyproc.c user space programs?
Yes

Can you help me write the testprocinfo.c and dummyproc.c user programs?

Here are complete implementations for both testprocinfo.c and dummyproc.c.  
PDF
1. user/testprocinfo.c

This program calls getpinfo() and prints process stats in a clean, tabular format, ignoring processes with pid == 0:  
PDF
C

#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/pstat.h"

int
main(int argc, char *argv[])
{
  struct pstat st;

  if (getpinfo(&st) < 0) {
    printf("testprocinfo: getpinfo failed\n");
    exit(1);
  }

  printf("PID\tIn Use\tIn Q\tWaiting time\tRunning time\tTimes sched.\tOrig. tix\tCurr. tix\tq0 ticks\tq1 ticks\n");
  printf("------------------------------------------------------------------------------------------------------------------------\n");

  for (int i = 0; i < NPROC; i++) {
    // Do not print processes whose PID is 0 or inactive process slots
    if (st.pid[i] == 0 || st.inuse[i] == 0)
      continue;

    printf("%d\t%d\t%d\t%d\t\t%d\t\t%d\t\t%d\t\t%d\t\t%d\t\t%d\n",
           st.pid[i],
           st.inuse[i],
           st.inQ[i],
           st.waiting_time[i],
           st.running_time[i],
           st.times_scheduled[i],
           st.tickets_original[i],
           st.tickets_current[i],
           st.queue_ticks[i][0],
           st.queue_ticks[i][1]);
  }

  exit(0);
}

2. user/dummyproc.c

This program accepts <tickets> and <iterations>, sets its ticket count via settickets(), and forks a child process that periodically calls sleep(1) to trigger voluntary yield promotions (PROMO):  
PDF
C

#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  if (argc < 3) {
    printf("Usage: dummyproc <tickets> <iterations>\n");
    exit(1);
  }

  int tickets = atoi(argv[1]);
  // Use uint64/long to prevent 32-bit overflow on large iteration values
  uint64 iterations = atoi(argv[2]);

  if (settickets(tickets) < 0) {
    printf("dummyproc: settickets failed\n");
    exit(1);
  }

  int pid = fork();

  if (pid < 0) {
    printf("dummyproc: fork failed\n");
    exit(1);
  }

  if (pid == 0) {
    // Child process: performs work and periodically sleeps to trigger PROMO
    for (uint64 i = 0; i < iterations; i++) {
      if (i % (iterations / 10 + 1) == 0) {
        sleep(1); // Voluntarily yields CPU before time slice expires
      }
    }
    exit(0);
  } else {
    // Parent process: CPU-bound loop to consume time slices and trigger DEMO
    for (uint64 i = 0; i < iterations; i++) {
      volatile uint64 x = i * i; // Heavy computation dummy work
      (void)x;
    }
    wait(0);
  }

  exit(0);
}

3. Update Makefile

Add both binaries to UPROGS in your Makefile:  
PDF
Makefile

UPROGS=\
  $U/_cat\
  $U/_echo\
  $U/_forktest\
  $U/_grep\
  $U/_init\
  $U/_kill\
  $U/_ln\
  $U/_ls\
  $U/_mkdir\
  $U/_rm\
  $U/_sh\
  $U/_stressfs\
  $U/_usertests\
  $U/_grind\
  $U/_wc\
  $U/_zombie\
  $U/_dummyproc\
  $U/_testprocinfo\

Want me to guide you through creating and testing the .patch file for submission?
Yes

Convert the assignment pdf to md. Remove unnecessary infos too

Here is the complete assignment specification converted into clean, concise Markdown format, omitting non-essential introductory or submission text while retaining all technical requirements and parameters.  
PDF
xv6 Scheduling: MLFQ with Lottery Scheduling and Aging
Overview

Replace the default round-robin scheduler in xv6 with a two-level Multilevel Feedback Queue (MLFQ) scheduler:  
PDF

    Queue 0 (Top Queue): Lottery scheduling.  
    PDF

    Queue 1 (Bottom Queue): Plain Round-Robin.  
    PDF

    Aging: Promotes processes waiting too long in Queue 1 to prevent starvation.  
    PDF

Specification Parameters

Define the following macros in kernel/param.h:  
PDF
Macro	Value	Description
TIME_LIMIT_0	2	

Time slice (ticks) for processes in Queue 0.  
PDF
TIME_LIMIT_1	4	

Time slice (ticks) for processes in Queue 1.  
PDF
WAIT_THRESH	6	

Waiting threshold (ticks) before aging promotes a process to Queue 0.  
PDF
DEFAULT_TICKETS	10	

Default ticket count for newly created processes.  
PDF
Scheduler Rules & Behavior

    New Processes: Start in Queue 0.  
    PDF

    Top-Down Search: Search starts from Queue 0. If Queue 0 contains RUNNABLE processes, schedule via Lottery. If Queue 0 is empty, search Queue 1 and schedule via Round-Robin.  
    PDF+ 2

    Queue Transitions:

        Demotion: Exceeding the queue's time slice (ticks_consumed >= TIME_LIMIT) pushes the process to Queue 1.  
        PDF

        Voluntary Yield Promotion: Voluntarily yielding control before time slice expiration (e.g., calling sleep) promotes the process to Queue 0.  
        PDF

        Aging Promotion: Waiting in Queue 1 for more than WAIT_THRESH ticks promotes the process to Queue 0.  
        PDF

    Lottery Algorithm (Queue 0):

        Sum tickets_current of all RUNNABLE Queue 0 processes (N).  
        PDF

        If all active Queue 0 processes have 0 tickets, reset all Queue 0 processes back to tickets_original.  
        PDF

        Draw a uniform random integer r∈[0,N).  
        PDF

        Accumulate tickets until running sum >r to select the winner.  
        PDF

        Decrement winner's tickets_current by 1.  
        PDF

Data Structures

Create kernel/pstat.h:  
PDF
C

#ifndef _PSTAT_H_
#define _PSTAT_H_

#include "param.h"

struct pstat {
  int pid[NPROC];              // Process ID of each process
  int inuse[NPROC];            // Whether slot is in use (1 or 0)
  int inQ[NPROC];              // Current queue (0 or 1)
  int waiting_time[NPROC];     // Ticks spent waiting before being scheduled
  int running_time[NPROC];     // Times scheduled before time slice expired
  int times_scheduled[NPROC];  // Total times scheduled
  int tickets_original[NPROC]; // Originally assigned tickets
  int tickets_current[NPROC];  // Currently held tickets
  uint queue_ticks[NPROC][2];  // Total ticks spent in each queue
};

#endif // _PSTAT_H_

System Calls
1. settickets
C

int settickets(int number);

    Sets tickets of calling process to number.  
    PDF

    Returns 0 on success, -1 if number < 1 (resets/retains DEFAULT_TICKETS).  
    PDF

2. getpinfo
C

int getpinfo(struct pstat *st);

    Populates struct pstat with current process metrics.  
    PDF

    Returns 0 on success, -1 on invalid/NULL pointer.  
    PDF

Special Requirements

    Fork Inheritance: Child processes inherit parent's tickets_original count.  
    PDF

    Single Core: Set CPUS := 1 in Makefile.  
    PDF

    Logging Formats:

        Demotion: DEMO: Process <pid> (<name>) ran for <ticks> time ticks, demoted to queue 1

          
        PDF

        Voluntary Promotion: PROMO: Process <pid> (<name>) ran for <ticks> time ticks, promoted to queue 0

          
        PDF

        Aging Boost: BOOST: Process <pid> (<name>) waited for <ticks> ticks, promoted to queue 0

          
        PDF

        Lottery Result: LOTTERY: Process <pid> (<name>) won in queue 0 with tickets <tickets>

          
        PDF

User Programs

    dummyproc <tickets> <iterations>: Sets tickets, performs work, and forks a child that periodically calls sleep to trigger voluntary yield promotions.  
    PDF

    testprocinfo: Calls getpinfo and displays running statistics in tabular format (excluding pid == 0).  
    PDF

How to test

Testing your MLFQ scheduler implementation in xv6 involves compiling the system, running user-space test utilities, verifying system call behaviors, and inspecting real-time scheduler logs.  
PDF

Here is the step-by-step workflow to test your code:
Step 1: Ensure Correct Configuration in Makefile

Before building, verify that xv6 is restricted to a single CPU core as specified in the assignment:  
PDF

    Open Makefile and confirm CPUS is set to 1:  
    PDF
    Makefile

    CPUS := 1

    Make sure your test programs are included under UPROGS in Makefile:  
    PDF
    Makefile

    $U/_dummyproc\
    $U/_testprocinfo\

Step 2: Compile and Launch QEMU

Clean up any previous build files and launch xv6:  
PDF
Bash

make clean
make qemu

If compilation succeeds without errors or warnings, the xv6 shell ($) will open.
Step 3: Run Basic Diagnostic Tests
1. Check Initial System Process Info

Run testprocinfo immediately after booting to confirm getpinfo reads system statistics correctly:  
PDF
Bash

$ testprocinfo

    Expected Result: It should print a table displaying active processes like init and sh. Confirm that processes with PID = 0 are excluded and all initial ticket values default to 10 (DEFAULT_TICKETS).  
    PDF+ 1

2. Test Ticket Assignment (settickets) and Lottery Distribution

Run multiple instances of dummyproc with different ticket allocations in the background:  
PDF
Bash

$ dummyproc 30 1000000000 &
$ dummyproc 10 1000000000 &
$ testprocinfo

    Expected Result:

        testprocinfo should list both dummyproc processes.  
        PDF

        The process given 30 tickets should show significantly higher q0 ticks and times_scheduled counts compared to the process given 10 tickets.  
        PDF

3. Test Voluntary Yield Promotion (PROMO) and Demotion (DEMO)

Run dummyproc in background mode:  
PDF
Bash

$ dummyproc 20 500000000 &

    Expected Logs:

        Parent Process (CPU-bound): Consumes its time slice (TIME_LIMIT_0 = 2) continuously, triggering DEMO logs to Queue 1:  
        PDF
        Plaintext

        DEMO: Process <pid> (dummyproc) ran for 2 time ticks, demoted to queue 1

        Child Process (I/O-bound with sleep): Voluntarily yields before exhausting its time slice, triggering PROMO logs back to Queue 0:  
        PDF
        Plaintext

        PROMO: Process <pid> (dummyproc) ran for 1 time ticks, promoted to queue 0

4. Test Aging Mechanism (BOOST)

To observe aging, launch a CPU-heavy process that forces low-priority processes to stay in Queue 1 long enough for waiting_time > 6 (WAIT_THRESH):  
PDF
Bash

$ dummyproc 10 2000000000 &

    Expected Log: Once a runnable process sits in Queue 1 for more than 6 ticks without being scheduled, the aging timer triggers a boost:  
    PDF
    Plaintext

    BOOST: Process <pid> (<name>) waited for 6 ticks, promoted to queue 0

Step 4: Validate Student Patch Generation

Before submitting, generate your patch file and verify it on a fresh copy of the repository:  
PDF

    Exit QEMU (Ctrl+A then X).

    Generate the patch using your 7-digit Student ID:  
    PDF
    Bash

    git add -A
    git diff HEAD > 2205192.patch

    Test applying the patch on a clean clone:  
    PDF
    Bash

    cd ..
    git clone https://github.com/shuaibw/xv6-riscv --depth=1 xv6-test
    cd xv6-test
    git apply ../2205192.patch
    make qemu

    Confirm make qemu builds cleanly and all tests work as expected.  
    PDF

Jjjwdjojdwibhqebljcdocqhvovhfqeqoeufvqwdvuovqfeufvqô

It looks like that text might be random keystrokes or corrupted input.

If you have a specific question or need help with code, an assignment, or troubleshooting, please let me know!


Gemini is AI and can make mistakes.
