# Online 2 System Call Problems and Solutions — xv6

This document compiles the System Call online exam problems found in the
`Online 2(System Call)` zip and provides a full, worked solution for each one.

**Problems covered:**

| Section | Topic | Type |
| --- | --- | --- |
| A1 | Bulk random numbers (`getRandomNumbers`) | System calls |
| A2 | Pseudo-random sampling (`choice`) | System calls |
| B1 | Single random number (`getRandomNumber`) | System calls |

> Syscall numbers below start at 22. In a fresh repo `SYS_fork..SYS_uptime` are
> 1..21, and there are no pre-existing `trace`/`history` syscalls, so we use
> consecutive numbers beginning at 22. Adjust if your repo already uses some of them.
> Files are relative to the repo root.
> All three problems share the same PRNG policy: calling the generator increases the
> kernel-global seed by 1 and uses/returns the new value. The seed persists across
> processes because it lives in kernel memory.

---

## Problem 1 — A1: Bulk Random Numbers

### Question

xv6 has no built-in pseudo-random number generator. Add a simple mechanism for
generating random numbers and returning them in bulk.

**Add 2 system calls:**

1. `setSeed(int seed)` — sets the seed for a pseudo-random number generator.
2. `getRandomNumbers(int n, struct randarray *out)` — generates the next `n`
   random numbers in ONE syscall (updating the internal state each time) and
   fills the user-supplied `out` struct. Merely calling a single-number
   generator `n` times from user space will not do — the loop of `n`
   increments must happen inside the kernel.

```c
struct randarray {
  int len;       // requested count (n), set by the caller
  int nums[15];  // generated numbers (output)
};
```

**Policy:** each generated number increases the seed by 1 and returns the new
value. You may safely assume `n < 15`.

**Add 2 user commands:**

1. `seed s`
2. `next n`

### Sample I/O

```
$ seed 2
The seed has been set to 2
$ next 2
Next random numbers are [3, 4]
$ next 3
Next random numbers are [5, 6, 7]
$ seed 12
The seed has been set to 12
$ next 1
Next random numbers are [13]
$ next 4
Next random numbers are [14, 15, 16, 17]
```

### Solution

#### 1. `kernel/syscall.h`

```c
#define SYS_setSeed          22
#define SYS_getRandomNumbers 23
```

#### 2. `kernel/sysproc.c`

```c
#include "syscall.h"
#include "spinlock.h"
#include "string.h"

static int seed_value = 0;   // kernel-global PRNG state

struct randarray {
  int len;
  int nums[15];
};

uint64
sys_setSeed(void)
{
  int z;
  if(argint(0, &z) < 0)
    return -1;
  seed_value = z;
  return 0;
}

uint64
sys_getRandomNumbers(void)
{
  struct randarray ubuf;
  uint64 out_addr;
  int n, i;

  if(argint(0, &n) < 0 || argaddr(1, &out_addr) < 0)
    return -1;
  if(n < 0 || n > 15)
    return -1;

  // The whole batch is generated inside the kernel: this is the graded
  // behavior (Task 2). A user-space loop of n single calls is not accepted.
  ubuf.len = n;
  for(i = 0; i < n; i++){
    seed_value++;              // increase seed by 1
    ubuf.nums[i] = seed_value; // ... and return the new value
  }

  if(copyout(myproc()->pagetable, out_addr, (char *)&ubuf, sizeof(ubuf)) < 0)
    return -1;
  return 0;
}
```

#### 3. `kernel/syscall.c`

Add extern declarations and array entries:

```c
extern uint64 sys_setSeed(void);
extern uint64 sys_getRandomNumbers(void);
```

```c
[SYS_setSeed]          sys_setSeed,
[SYS_getRandomNumbers] sys_getRandomNumbers,
```

#### 4. `user/user.h`

```c
struct randarray {
  int len;
  int nums[15];
};

int setSeed(int);
int getRandomNumbers(int, struct randarray*);
```

#### 5. `user/usys.pl`

```perl
entry("setSeed");
entry("getRandomNumbers");
```

#### 6. `user/seed.c` and `user/next.c`

`user/seed.c`:

```c
#include "kernel/types.h"
#include "kernel/param.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  if(argc < 2){
    fprintf(2, "Usage: seed <s>\n");
    exit(1);
  }
  setSeed(atoi(argv[1]));
  printf("The seed has been set to %d\n", atoi(argv[1]));
  exit(0);
}
```

`user/next.c`:

```c
#include "kernel/types.h"
#include "kernel/param.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  struct randarray out;
  int i, n;

  if(argc < 2){
    fprintf(2, "Usage: next <n>\n");
    exit(1);
  }
  n = atoi(argv[1]);
  if(n < 0 || n > 15){
    fprintf(2, "next: n must satisfy 0 <= n < 15\n");
    exit(1);
  }

  out.len = n;
  if(getRandomNumbers(n, &out) < 0){
    fprintf(2, "next: getRandomNumbers failed\n");
    exit(1);
  }

  printf("Next random numbers are [");
  for(i = 0; i < n; i++){
    if(i > 0) printf(", ");
    printf("%d", out.nums[i]);
  }
  printf("]\n");
  exit(0);
}
```

#### 7. `Makefile`

```make
	$U/_seed\
	$U/_next\
```

### Tips

- The seed is a kernel-global (`static int` in `sysproc.c`), so it survives
  across separate `seed` / `next` invocations — that is why `next 3` after
  `next 2` continues at 5, not 3.
- Generate all `n` numbers in the single syscall loop; the examiner checks that
  the state advances by `n` per `next n` call.
- Validate `n` (`0 <= n <= 15`) so a bad length can never overflow `nums[15]`.

### Flow Explanation (to the Teacher)

**Goal:** `getRandomNumbers(int n, struct randarray *out)` advances the
kernel seed `n` times in one trap and hands all `n` new values back to user
space.

#### 1. User command → trap

> "User runs `next 2`. In `user/next.c`, `main()` calls
> `getRandomNumbers(n, &out)`."

The wrapper comes from `user/usys.pl` (`entry("getRandomNumbers");`), which
generates a stub in `usys.S`. It does `ecall` → a **trap** into kernel mode, with:

- `a0` = `n` (the count)
- `a1` = pointer to `&out` (the output struct)

Those register values are saved in the process `trapframe`.

#### 2. Dispatch

> "The kernel reads the syscall number and indexes the `syscalls[]` table in
> `kernel/syscall.c` → `[SYS_getRandomNumbers] sys_getRandomNumbers`."

`SYS_getRandomNumbers` is **23** in `kernel/syscall.h` (second free number after 21).

#### 3. Read the two args

```c
if(argint(0, &n) < 0 || argaddr(1, &out_addr) < 0)
  return -1;
```

> "`argint` pulls the count straight out of the trapframe. `argaddr` pulls the
> raw *user virtual address* of the struct — I must NOT dereference it directly,
> it points into user memory."

#### 4. Compute — the batch loop (the graded part)

```c
ubuf.len = n;
for(i = 0; i < n; i++){
  seed_value++;
  ubuf.nums[i] = seed_value;
}
```

> "Each iteration bumps the persistent kernel seed and stores the new value, so
> one syscall advances the state by exactly `n`. For `seed 2` + `next 2`: the
> seed goes 2→3→4 and `nums = [3, 4]`."

#### 5. Send the result back

```c
if(copyout(myproc()->pagetable, out_addr, (char *)&ubuf, sizeof(ubuf)) < 0)
  return -1;
return 0;
```

> "`copyout` writes the whole struct from kernel memory back into the user's
> `out`. Return `0` = success. (The exam text says the call 'returns a pointer
> to an object' — since user space cannot dereference kernel pointers, the xv6
> way to do that is this in-place `copyout`, same pattern as the `sample`
> problem.)"

#### 6. Back in user space

```c
printf("Next random numbers are [");
for(i = 0; i < n; i++){
  if(i > 0) printf(", ");
  printf("%d", out.nums[i]);
}
printf("]\n");
```

> "`out.nums[]` is now filled; the user program prints it in the exact sample
> format."

**Registration recap:** syscall.h (`#define 22/23`) · sysproc.c (handler +
`static int seed_value`) · syscall.c (`extern` + table entries) · user.h
(prototype + struct) · usys.pl (`entry`) · plus `seed.c`, `next.c` and the
Makefile `$U/_seed\` / `$U/_next\` lines.

---

## Problem 2 — A2: Pseudo-Random Sampling (`choice`)

### Question

xv6 has no built-in pseudo-random number generator. Add a simple mechanism for
generating random numbers and selecting a random element from an array, similar
to the Python function `random.choice()`.

**Add 2 system calls:**

1. `setSeed(int)` — sets the seed for a pseudo-random number generator.
2. `choice(struct array*)` — returns the randomly selected number from the
   array and updates the internal state.

```c
struct array{
  int len;        // length of array
  int array[15];  // array elements
};
```

**Policy:** calling `choice` first increases the seed by 1, and the selected
index is `seed % array->len`. The return value of the syscall is the selected
element. You may safely assume the array length is at most 15.

**Add 2 user commands:**

1. `seed n`
2. `choice len [the_array elements]`

### Sample I/O

```
$ seed 2
The seed has been set to 2
$ choice 3 1 2 3
Randomly selected element is 1
$ choice 5 10 20 30 40 50
Randomly selected element is 50
$ seed 12
The seed has been set to 12
$ choice 3 6 7 8
Randomly selected element is 7
$ choice 1 5
Randomly selected element is 5
```

Check the policy against the samples: after `seed 2`, seed→3, `3 % 3 = 0` →
`1`; next call seed→4, `4 % 5 = 4` → `50`. After `seed 12`, seed→13,
`13 % 3 = 1` → `7`; then seed→14, `14 % 1 = 0` → `5`.

### Solution

#### 1. `kernel/syscall.h`

```c
#define SYS_setSeed 22
#define SYS_choice  23
```

#### 2. `kernel/sysproc.c`

```c
#include "syscall.h"
#include "spinlock.h"
#include "string.h"

static int seed_value = 0;

struct array {
  int len;
  int array[15];
};

uint64
sys_setSeed(void)
{
  int z;
  if(argint(0, &z) < 0)
    return -1;
  seed_value = z;
  return 0;
}

uint64
sys_choice(void)
{
  struct array uarr;
  uint64 arr_addr;
  int idx;

  if(argaddr(0, &arr_addr) < 0)
    return -1;
  if(copyin(myproc()->pagetable, (char *)&uarr, arr_addr, sizeof(uarr)) < 0)
    return -1;
  if(uarr.len <= 0 || uarr.len > 15)
    return -1;

  seed_value++;                        // first increase the seed by 1
  idx = seed_value % uarr.len;         // selected index
  return uarr.array[idx];              // return the selected element
}
```

#### 3. `kernel/syscall.c`

```c
extern uint64 sys_setSeed(void);
extern uint64 sys_choice(void);
```

```c
[SYS_setSeed] sys_setSeed,
[SYS_choice]  sys_choice,
```

#### 4. `user/user.h`

```c
struct array {
  int len;
  int array[15];
};

int setSeed(int);
int choice(struct array*);
```

#### 5. `user/usys.pl`

```perl
entry("setSeed");
entry("choice");
```

#### 6. `user/seed.c` and `user/choice.c`

`user/seed.c`: same as Problem 1 (`setSeed(atoi(argv[1]))`, print
`The seed has been set to %d`).

`user/choice.c`:

```c
#include "kernel/types.h"
#include "kernel/param.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  struct array arr;
  int i, picked;

  // choice len [elements...]
  if(argc < 3){
    fprintf(2, "Usage: choice <len> <elements...>\n");
    exit(1);
  }
  arr.len = atoi(argv[1]);
  if(arr.len <= 0 || arr.len > 15 || argc < 2 + arr.len){
    fprintf(2, "choice: bad length\n");
    exit(1);
  }
  for(i = 0; i < arr.len; i++)
    arr.array[i] = atoi(argv[2 + i]);

  picked = choice(&arr);
  printf("Randomly selected element is %d\n", picked);
  exit(0);
}
```

#### 7. `Makefile`

```make
	$U/_seed\
	$U/_choice\
```

### Tips

- Unlike Problem 1, no `copyout` is needed: the struct is only read
  (`copyin`), and the answer comes back as the syscall return value (in `a0`).
- Guard `len <= 0` — `seed % 0` would divide by zero in the kernel.
- Negative seeds still work with this policy, but C's `%` can yield a negative
  index for negative seeds; the samples only use non-negative seeds.

### Flow Explanation (to the Teacher)

**Goal:** `choice(struct array*)` reads the caller's array, bumps the seed
once, and returns `array[seed % len]`.

#### 1. User command → trap

> "User runs `choice 3 1 2 3`. `user/choice.c` packs `len = 3`,
> `array = {1, 2, 3}` into the struct and calls `choice(&arr)`."

`a0` = user pointer to the struct.

#### 2. Bring the struct in

```c
if(argaddr(0, &arr_addr) < 0)
  return -1;
if(copyin(myproc()->pagetable, (char *)&uarr, arr_addr, sizeof(uarr)) < 0)
  return -1;
```

> "`copyin` copies the whole struct from user memory into the kernel-local
> `uarr`, validating the address. I never touch the user pointer directly."

#### 3. Compute and return

```c
seed_value++;
idx = seed_value % uarr.len;
return uarr.array[idx];
```

> "One increment, one modulo, return the element — the return value lands in
> `a0`, so user space gets it as the function's return value. For
> `seed 2` + `choice 3 1 2 3`: seed 2→3, `3 % 3 = 0`, returns `array[0] = 1`."

---

## Problem 3 — B1: Single Random Number

### Question

xv6 has no built-in pseudo-random number generator. Add a simple mechanism for
generating random numbers.

**Add 2 system calls:**

1. `setSeed(int)` — sets the seed for a pseudo-random number generator.
2. `getRandomNumber()` — returns the next random number and updates the
   internal state.

**Policy:** calling `getRandomNumber()` increases the seed by 1 and returns it.

**Add 2 user commands:**

1. `seed n`
2. `next`

### Sample I/O

```
$ seed 2
The seed has been set to 2
$ next
Next random number is 3
$ next
Next random number is 4
$ seed 12
The seed has been set to 12
$ next
Next random number is 13
$ next
Next random number is 14
```

### Solution

#### 1. `kernel/syscall.h`

```c
#define SYS_setSeed         22
#define SYS_getRandomNumber 23
```

#### 2. `kernel/sysproc.c`

```c
#include "syscall.h"
#include "spinlock.h"
#include "string.h"

static int seed_value = 0;

uint64
sys_setSeed(void)
{
  int z;
  if(argint(0, &z) < 0)
    return -1;
  seed_value = z;
  return 0;
}

uint64
sys_getRandomNumber(void)
{
  seed_value++;          // increase the seed by 1 and return it
  return seed_value;
}
```

#### 3. `kernel/syscall.c`

```c
extern uint64 sys_setSeed(void);
extern uint64 sys_getRandomNumber(void);
```

```c
[SYS_setSeed]         sys_setSeed,
[SYS_getRandomNumber] sys_getRandomNumber,
```

#### 4. `user/user.h`

```c
int setSeed(int);
int getRandomNumber(void);
```

#### 5. `user/usys.pl`

```perl
entry("setSeed");
entry("getRandomNumber");
```

#### 6. `user/seed.c` and `user/next.c`

`user/seed.c`: same as Problem 1.

`user/next.c`:

```c
#include "kernel/types.h"
#include "kernel/param.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  (void)argc;
  (void)argv;
  printf("Next random number is %d\n", getRandomNumber());
  exit(0);
}
```

#### 7. `Makefile`

```make
	$U/_seed\
	$U/_next\
```

### Tips

- This is the minimal version of the family: no structs, no `copyin`/`copyout`
  — the answer travels back in the return register.
- The same kernel-global-seed reasoning as Problem 1 applies: separate `seed`
  and `next` processes share state through kernel memory.
- `next` takes no arguments; ignore `argc` to keep `-Werror` builds clean.

---

## General Submission (all problems)

```bash
git add --all
git diff HEAD > ../{studentID}.patch
```

Remember to run `make clean` before rebuilding, and verify against the sample I/O.
