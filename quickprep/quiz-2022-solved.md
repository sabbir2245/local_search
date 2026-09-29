# 2022 Quiz (CSE 314) — Problems + Solutions (syllabus-filtered)

Source: `~/Downloads/2022quiz.pdf` (45 points, 40 min).
Syllabus basis = `quickprep/` contents: `bash/` (bash scripting), `ipc/` (pthreads/semaphores/sync), `scheduling/` + `xv6/` (scheduler, wait/sleep, trap, VM patch work).

Solved below: Q2, Q3, Q4, Q5*, Q6, Q7, Part-B Q1–Q4, Q6–Q8.
Skipped with reason: Q1(a)(b) needs Figure 1 (not in PDF text) + page-table geometry not in quickprep; Part-B Q5 needs the TLB-assignment page-replacement setup (not in quickprep).

---

## Q2 (5) — Change file extension [bash — IN SYLLABUS]

**Problem.** Write `sample.sh` taking two args; `./sample.sh .dat .txt` renames every `*.dat` in CWD to `*.txt`.

**Solution.**
```bash
#!/bin/bash
# usage: ./sample.sh .dat .txt
if [ $# -ne 2 ]; then echo "Usage: $0 <old_ext> <new_ext>"; exit 1; fi
old="$1"; new="$2"
for f in *"$old"; do
  [ -e "$f" ] || continue        # no match -> glob stays literal
  [ -f "$f" ] || continue
  base="${f%$old}"               # strip old suffix
  mv -- "$f" "$base$new"
done
```
Notes: `"$f"` quoted for spaces; `--` guards names starting with `-`; `strip` only touches the suffix.

## Q3 (5) — Shell one-liners [bash — IN SYLLABUS]

**Problem.** (a) Print lines 15–22 of a 100+ line file. (b) Save all `grep` usages from terminal history to `grep.txt`.

**Solution.**
```bash
# (a) any one of:
sed -n '15,22p' file.txt
awk 'NR>=15 && NR<=22' file.txt
head -n 22 file.txt | tail -n 8
# (b)
history | grep -w grep > grep.txt
# (in zsh without history cmd: fc -l 1 | grep -w grep > grep.txt)
```

## Part-B Q1 (1) — `exec <` redirection [bash — IN SYLLABUS]

**Problem.** `exec < file1; exec < file2; exec < file3; read line` — where does `read` read from?
**Answer: (d) only file 3.** Each `exec < f` replaces fd 0; only the last one survives.

## Part-B Q2 (1) — quoting [bash — IN SYLLABUS]

**Problem.** `os=linux; echo 1.$os 2."$os" 3.'$os' 4.os`
**Answer: (c) `1.linux 2.linux 3.$os 4.os`.** Unquoted/double-quoted expand; single quotes don't; bare `os` is literal.

## Q4 (6) — Stairway / threads + semaphores [IPC — IN SYLLABUS]

**Problem.** X steps, ≤1 person per step per direction (≤2X total), no overtaking, 10 ms per step, one thread per person.
(a) Why do two global counting semaphores (`Sem_upward = Sem_downward = X`) fail?
(b) What semaphores model it correctly?

**Solution.**
(a) Global counters only cap the *total* per direction. They cannot enforce *which step* is free, so two upward persons can take the same step (collision), an upward and downward person can deadlock head-on inside one step with no passing rule, overtaking/FIFO is unenforced, and "next step empty" is never tested — the exact per-step state the model requires is invisible to the semaphores.
(b) Per-step exclusion is required. Minimal correct set:
- `mutex[i]`, binary, init 1, for `i = 1..X` — one per step; a person must `wait(mutex[next])` before entering the next step and `signal(mutex[prev])` after leaving. Guarantees ≤1 person per step per direction slot and no overtaking past a held step.
- Optionally one counting semaphore `empty_up = X`, `empty_down = X` to bound total occupancy per direction (admission control), plus per-direction FIFO queues for fairness.
Usage: `wait(empty_dir); wait(mutex[next_step]); move; signal(mutex[prev_step]); signal(empty_dir)` (with the 10 ms hold inside the critical section). Overtaking is prevented because entry to each step is serialized per direction.

## Part-B Q3 (1) — thread sync API [IPC — IN SYLLABUS]

**Answer: (c) `pthread_join`.** `pthread_exit` terminates, `pthread_cancel` cancels, `pthread_self` returns own id — none synchronize two threads; `pthread_join` (with mutex/cond/sem) does.

## Part-B Q4 (1) — readers–writers [IPC — IN SYLLABUS]

**Answer: (b) writers.** Writers get exclusive access; readers may share among themselves (first/second readers–writers variants differ only in priority, writers always exclude everyone while writing).

## Q6 (3) — removing `sti()` from scheduler [scheduling/xv6 — IN SYLLABUS]

**Problem.** xv6 `scheduler()` calls `sti()` (enable interrupts) each loop; student removes it and "it works". Good idea?
**Answer: No.** The scheduler must run with interrupts enabled so the timer interrupt can preempt a RUNNING process and return control to the scheduler. Without `sti()`: on one CPU the first scheduled process never gets timer-preempted (CPU hog / hang if it never yields/sleeps); on SMP, lost wakeups and deadlock when holding `ptable.lock` with interrupts disabled; "it works" only for trivial runs that voluntarily yield. Keep `sti()` before `acquire(&ptable.lock)`.

## Q7 (4) — reordered `wait()` / lost wakeup [xv6 — IN SYLLABUS]

**Problem.** Original checks exited-child *before* `sleep(proc)`; rewrite sleeps *first*, then checks. What breaks?
**Answer: Lost wakeup → parent sleeps forever.** If a child exits after the `has no children` check but before `sleep()`, its `wakeup(parent)` fires while the parent is still awake; the rewritten code then sleeps with no pending wakeup and no exited child noticed until another (maybe never) event. Original order + holding `ptable.lock` across check-and-sleep makes the check/sleep atomic, closing the race.

## Q5 (8) — TLB on xv6 [xv6 — IN SYLLABUS, brief]

**Problem.** Add a TLB atop the paging assignment; name files + pseudocode.
**Answer.** Touch `kernel/vm.c` (`walk`, `mappages`, `switchuvm`/`switchkvm`), `kernel/trap.c` (page-fault path), `kernel/defs.h` + `kernel/proc.h` (TLB struct), `kernel/memlayout.h` if ASIDs added.
```c
// per-cpu: struct { uint vpn; uint ppn; int valid; } tlb[N];
walkaddr(pgdir, va):
  if tlb_lookup(va, &pa) return pa;      // hit
  pte = walk(pgdir, va);                  // existing walk
  if (!pte || !(*pte & PTE_P)) pagefault();
  tlb_insert(va, PTE_ADDR(*pte));         // FIFO/random evict
  return PTE_ADDR(*pte) | (va & 0xFFF);
// flush on switchuvm()/switchkvm() and on mappages() unmap; shootdown omitted (single-core xv6).
```

## Part-B Q6/Q7/Q8 (1 each) — xv6 facts [xv6 — IN SYLLABUS]

- **Q6 `panic()`:** **(c) kernel bug.** User bugs → killed signals; spurious interrupts recover; bad syscall number → error return.
- **Q7 false statement:** **(c) "all pages allocated at start".** xv6 allocates lazily (`allocuvm` on demand/`exec`/sbrk); (a) `PTXSHIFT = 12` is true, (b) per-process page directory is true.
- **Q8 `trapret`:** **(d) allocating new process, after `forkret`.** `forkret` finishes setup then `trapret` drops to user space on first schedule (also used on every trap return path via `trapret` in newer xv6, but among the choices (d) is the intended one).

---

## Skipped (out of quickprep syllabus or missing data)

- **Q1(a)(b) paging math** — needs Figure 1 (m/n/p/k/i/j geometry) which is an image, absent from the PDF text; page-table-level design is not covered by any `quickprep/` file. Skipped.
- **Part-B Q5 page-fault count** — depends on the custom `MAX_PSYC_PAGES=4` assignment setup, not present in `quickprep/`. Skipped.
