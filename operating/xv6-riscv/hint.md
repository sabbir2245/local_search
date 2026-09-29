To solve Assignment 2 (xv6 System Calls), you need to register two new system calls—trace and history—modify kernel tracking logic in kernel/syscall.c, enforce synchronization using spinlocks, and add two user utilities (user/trace.c and user/history.c).  
Phase 1: Register System Calls in Kernel & User Space
Step 1: Assign System Call Numbers

Edit kernel/syscall.h to add unique numbers for SYS_trace and SYS_history:  
C

#define SYS_trace   22
#define SYS_history 23

Step 2: Declare User-Space Prototypes

In user/user.h, declare the syscall_stat structure and function prototypes:
C

struct syscall_stat {
  char syscall_name[16];
  int count;
  int accum_time;
};

int trace(int);
int history(int, struct syscall_stat*);

Step 3: Add Stubs and Makefile Targets

    In user/usys.pl, append the stub generation entries:
    Perl

    entry("trace");
    entry("history");

    In Makefile, add $U/_trace\ and $U/_history\ to UPROGS.  

Phase 2: Implement Task 1 (trace)
Step 1: Update Process Control Block

Modify struct proc in kernel/proc.h to store the system call number being traced for that process:
C

struct proc {
  // ... existing fields ...
  int trace_sys_num; // Stores system call number to trace (0 if none)
};

Initialize p->trace_sys_num = 0; inside allocproc() in kernel/proc.c.
Step 2: Implement sys_trace Handler

In kernel/sysproc.c, fetch the integer argument and store it in the calling process's PCB:  
C

uint64
sys_trace(void)
{
  int sys_num;
  argint(0, &sys_num);
  myproc()->trace_sys_num = sys_num;
  return 0;
}

Step 3: Update System Call Dispatcher & Printing

In kernel/syscall.c:

    Declare system call names and argument format tables:
    C

    extern uint64 sys_trace(void);
    extern uint64 sys_history(void);

    // Map syscall numbers to handlers
    static uint64 (*syscalls[])(void) = {
      // ... existing syscalls ...
      [SYS_trace]   sys_trace,
      [SYS_history] sys_history,
    };

    // System call names array
    static char *syscall_names[] = {
      [SYS_fork] "fork", [SYS_exit] "exit", [SYS_wait] "wait", [SYS_pipe] "pipe",
      [SYS_read] "read", [SYS_kill] "kill", [SYS_exec] "exec", [SYS_fstat] "fstat",
      [SYS_chdir] "chdir", [SYS_dup] "dup", [SYS_getpid] "getpid", [SYS_sbrk] "sbrk",
      [SYS_sleep] "sleep", [SYS_uptime] "uptime", [SYS_open] "open", [SYS_write] "write",
      [SYS_mknod] "mknod", [SYS_unlink] "unlink", [SYS_link] "link", [SYS_mkdir] "mkdir",
      [SYS_close] "close", [SYS_trace] "trace", [SYS_history] "history"
    };

    Implement a helper function in kernel/syscall.c to read and format system call arguments dynamically from registers (a0, a1, a2) based on each system call's signature.

    Modify syscall() in kernel/syscall.c to print tracing details when the system call returns if num == p->trace_sys_num:  
    C

    void
    syscall(void)
    {
      int num;
      struct proc *p = myproc();

      num = p->trapframe->a7;
      if(num > 0 && num < NELEM(syscalls) && syscalls[num]) {
        // Save register arguments before execution if needed for tracing

        p->trapframe->a0 = syscalls[num](); // Execute system call

        if(p->trace_sys_num == num) {
          // Print trace: pid: %d, syscall: %s, args: (...), return: %d[cite: 1]
          printf("pid: %d, syscall: %s, args: (...", p->pid, syscall_names[num]);
          // Print formatted args here
          printf("), return: %d\n", p->trapframe->a0);
        }
      }
    }

Step 4: Add User Command user/trace.c

Create user/trace.c based on the provided code in the assignment document[cite: 1]:
C

#include "kernel/param.h"
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[]) {
  int i;
  char *nargv[MAXARG];

  if(argc < 3 || (argv[1][0] < '0' || argv[1][0] > '9')) {
    fprintf(2, "Usage: %s sys_call_num command\n", argv[0]);
    exit(1);
  }

  if (trace(atoi(argv[1])) < 0) {
    fprintf(2, "%s: trace failed\n", argv[0]);
    exit(1);
  }

  for(i = 2; i < argc && i < MAXARG; i++) {
    nargv[i-2] = argv[i];
  }
  nargv[i-2] = 0;

  exec(nargv[0], nargv);
  exit(0);
}

Phase 3: Implement Task 2 (history)
Step 1: Thread-Safe Statistics in Kernel

In kernel/syscall.c, define aggregated metrics and a spinlock for concurrency safety:
C

#include "spinlock.h"

struct spinlock sys_stat_lock;
int sys_call_counts[30];
int sys_call_times[30];

void
syscallinit(void)
{
  initlock(&sys_stat_lock, "syscall_stat"); // Initialize spinlock
}

Call syscallinit() during system startup in kernel/main.c.
Step 2: Record System Call Execution Time & Counts

In kernel/syscall.c, measure time spent during execution using kernel ticks:
C

extern uint ticks; // From kernel/trap.c

void
syscall(void)
{
  int num;
  struct proc *p = myproc();

  num = p->trapframe->a7;
  if(num > 0 && num < NELEM(syscalls) && syscalls[num]) {
    uint start_ticks = ticks;

    p->trapframe->a0 = syscalls[num](); // Call system call

    uint end_ticks = ticks;

    // Concurrency protection using spinlock
    acquire(&sys_stat_lock);
    sys_call_counts[num]++;
    sys_call_times[num] += (end_ticks - start_ticks);
    release(&sys_stat_lock);

    // ... Handle Task 1 trace printing ...
  }
}

Step 3: Implement sys_history Kernel Handler

In kernel/sysproc.c, copy data to user memory using copyout():
C

uint64
sys_history(void)
{
  int sys_num;
  uint64 st_ptr; // User-space pointer to struct syscall_stat
  struct proc *p = myproc();
  struct syscall_stat st;

  argint(0, &sys_num);
  argaddr(1, &st_ptr);

  if(sys_num <= 0 || sys_num >= 30)
    return -1;

  acquire(&sys_stat_lock);
  safestrcpy(st.syscall_name, syscall_names[sys_num], sizeof(st.syscall_name));
  st.count = sys_call_counts[sys_num];
  st.accum_time = sys_call_times[sys_num];
  release(&sys_stat_lock);

  // Copy structure back to user space safely
  if(copyout(p->pagetable, st_ptr, (char *)&st, sizeof(st)) < 0)
    return -1;

  return 0;
}

Step 4: Create User Program user/history.c

Create user/history.c to fetch statistics and print them in user space:
C

#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

void print_stat(int sys_num) {
  struct syscall_stat st;
  if (history(sys_num, &st) == 0) {
    printf("%d: syscall: %s, #: %d, time: %d\n", sys_num, st.syscall_name, st.count, st.accum_time);
  }
}

int main(int argc, char *argv[]) {
  if (argc == 2) {
    // History for specific syscall
    int sys_num = atoi(argv[1]);
    print_stat(sys_num);
  } else {
    // History for all syscalls
    for (int i = 1; i <= 22; i++) {
      print_stat(i);
    }
  }
  exit(0);
}

Phase 4: Patch File Generation

    Test your solution inside xv6:
    Bash

    $ trace 15 grep hello README
    $ history 5
    $ history

    Generate your submission patch file using your 7-digit student ID:
    Bash

    git add --all
    git diff HEAD > studentID.patch
