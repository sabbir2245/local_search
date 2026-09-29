# xv6 system-call tracing and history — 2205040

This implementation adds the two system calls required by the assignment:

- `trace(syscall_number)` enables return-time tracing for exactly the process that invoked it.
- `history(syscall_number, stat)` returns the aggregate name, invocation count, and elapsed tick total for one system call.

## Kernel changes

`kernel/proc.h` gives each process a `trace_sys_num` field.  It is reset both when a process is allocated and when it is freed, so tracing cannot leak into a later user of a process-table slot.  The field is deliberately not copied in `fork()`: only the process that called `trace` is enabled.

`kernel/syscall.c` contains the syscall number-to-name map, one statistics record per syscall, and one spinlock per record.  The locks are initialized during boot by `syscall_stats_init()` from `main()`.  At syscall dispatch, the original register arguments are saved, ticks are sampled before and after the handler, and the corresponding record is updated while holding only that syscall's lock.  This avoids lost updates on multiple CPUs without serializing unrelated system calls.

The trace formatter handles every implemented syscall.  Integer arguments are printed as decimal, pointer arguments as xv6's full-width hexadecimal pointers, and pathname/program-name arguments are copied safely from user memory and printed as strings.  The trace line is printed after the handler returns, so it includes the true return value.

`sys_trace` validates and stores the selected number.  `sys_history` validates its number, snapshots the protected record, and uses `copyout` to return the user-mode structure.  Because accounting happens after a handler returns, a call to `history` reports the history accumulated before that particular call, matching the assignment example.

## User programs

`user/trace.c` parses a syscall number, calls `trace`, and `exec`s the requested command.  `user/history.c` accepts either one syscall number or no number; the latter queries and prints every syscall from `fork` through `history`.  Both programs are added to the file-system image by the Makefile.

All added or changed source sections are marked with `//====new code =====` as requested.

## Test result

Source-level checks passed: `gcc -fsyntax-only` compiled the four new/changed C translation units with warnings treated as errors, `perl -c user/usys.pl` passed, and `make -n fs.img` confirmed that both `_trace` and `_history` are included in the image build.

A full `make -j2` was attempted.  The local environment does not have a RISC-V cross compiler (or QEMU), so xv6 cannot be built or boot-tested here: Makefile toolchain discovery falls back to host `gcc`, which rejects RISC-V options and instructions.  With a configured `riscv64-unknown-elf-` or compatible toolchain, run `make qemu` and test, for example:

```
trace 15 grep hello README
trace 5 grep hello README
trace 7 echo hello
history 5
history
```
