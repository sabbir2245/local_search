# BVCS — Test Plan

All tests assume `bvcs` points to the script:

```bash
alias bvcs=/path/to/bvcs.sh
```

Run each test in a **fresh temporary directory** to avoid state leakage:

```bash
td=$(mktemp -d) && cd "$td"
```

---

## 1. `init`

| # | Test | Command | Expected output |
|---|------|---------|-----------------|
| 1.1 | Create repository | `bvcs init` | `Initialized empty BVCS repository.` |
| 1.2 | Directory layout | `ls -a .bvcs/` | Contains `.`, `..`, `HEAD`, `log`, `objects`, `staging` |
| 1.3 | Re-init (error) | `bvcs init` | `Error: BVCS repository already exists.` |

---

## 2. `add`

| # | Test | Command | Expected output |
|---|------|---------|-----------------|
| 2.1 | No arguments | `bvcs add` | `Error: No files specified.` |
| 2.2 | Stage files | `echo a > a.txt; echo b > b.txt; bvcs add a.txt b.txt` | `Staged: a.txt` then `Staged: b.txt` |
| 2.3 | Already staged | `bvcs add a.txt` | `Already staged: a.txt` |
| 2.4 | Missing file | `bvcs add ghost.txt` | `Error: 'ghost.txt' not found.` |
| 2.5 | Staging file contents | `cat .bvcs/staging` | `a.txt` then `b.txt` |
| 2.6 | Mixed valid + missing | `bvcs add a.txt ghost.txt b.txt` | Staged/error notices for each in order |

---

## 3. `status`

| # | Test | Prerequisite | Command | Expected output |
|---|------|-------------|---------|-----------------|
| 3.1 | Clean (no files) | `bvcs init` | `bvcs status` | `Nothing to commit, working tree clean.` |
| 3.2 | Staged + untracked | init, create `g.txt`, `w.txt`, `n.md`; add g.txt w.txt | `bvcs status` | Staged for commit with g.txt, w.txt; Untracked with n.md |
| 3.3 | All three categories | Manually create HEAD (see checkpoint) | `bvcs status` | Staged, Modified, and Untracked sections |
| 3.4 | Untracked file in subdir | `mkdir sub; echo x > sub/f.txt` after init | `bvcs status` | Untracked includes `sub/f.txt` |
| 3.5 | Exclude `.bvcs/` | Create a file inside `.bvcs/` | `bvcs status` | Never appears in untracked |

### Checkpoint for 3.3 (all three categories)

```bash
bvcs init
echo hello > greet.txt && echo world > world.txt
bvcs add greet.txt world.txt
echo "# Notes" > notes.md
mkdir -p .bvcs/objects/0001/files
cp greet.txt world.txt .bvcs/objects/0001/files/
echo "0001" > .bvcs/HEAD
> .bvcs/staging
echo "goodbye" >> greet.txt
bvcs add world.txt
bvcs status
```

Expected:

```
Staged for commit:
world.txt

Modified (not staged):
greet.txt

Untracked files:
notes.md
```

---

## 4. `commit`

| # | Test | Command | Expected output |
|---|------|---------|-----------------|
| 4.1 | No message | `bvcs commit` | `Error: Commit message required. Use -m "message".` |
| 4.2 | Empty message | `bvcs commit -m ""` | `Error: Commit message required. Use -m "message".` |
| 4.3 | Empty staging | `bvcs commit -m "msg"` | `Error: Nothing to commit.` |
| 4.4 | Success | add files then `bvcs commit -m "Initial commit"` | `[0001] Initial commit` then `N file(s) committed.` |
| 4.5 | HEAD updated | `cat .bvcs/HEAD` | `0001` |
| 4.6 | Log updated | `cat .bvcs/log` | `0001\|<timestamp>\|Initial commit` |
| 4.7 | Staging cleared | `cat .bvcs/staging` | (empty) |
| 4.8 | Snapshot created | `ls .bvcs/objects/0001/files/` | All committed files |
| 4.9 | Carry-forward | Commit twice; second snapshot contains first commit's files | Files from 0001 appear in 0002/files/ |
| 4.10 | Multiple files count | Commit 3 files | `3 file(s) committed.` |

---

## 5. `log`

| # | Test | Prerequisite | Command | Expected |
|---|------|-------------|---------|----------|
| 5.1 | No commits | `bvcs init` | `bvcs log` | `No commits yet.` |
| 5.2 | Single commit | One commit | `bvcs log` | One entry, blank line after |
| 5.3 | Multiple commits | Two commits | `bvcs log` | Most recent first; blank line after each |

Log format (exact):

```
commit 0002
Date:   2026-07-10 12:00:00
Message: Second commit

commit 0001
Date:   2026-07-10 11:00:00
Message: First commit
```

Note: blank line **after the last entry**.

---

## 6. `diff`

| # | Test | Command | Expected |
|---|------|---------|----------|
| 6.1 | No commits | `bvcs diff` | `Error: No commits yet.` |
| 6.2 | File not tracked | `bvcs diff ghost.txt` | `Error: 'ghost.txt' is not tracked.` |
| 6.3 | Modified file | add+commit, modify, `bvcs diff f.txt` | Unified diff output |
| 6.4 | Unchanged file | `bvcs diff f.txt` (no changes) | `f.txt: no changes.` |
| 6.5 | All files (no arg) | `bvcs diff` | Diff for every tracked file, none omitted |

Diff format (exact):

```
--- .bvcs/objects/0001/files/f.txt
+++ f.txt
@@ -1 +1 @@
-old
+new
```

No timestamps in `---`/`+++` headers.

---

## 7. `restore`

| # | Test | Command | Expected |
|---|------|---------|----------|
| 7.1 | No argument | `bvcs restore` | `Error: No file specified.` |
| 7.2 | No commits | `bvcs restore f.txt` | `Error: No commits yet.` |
| 7.3 | File not in snapshot | commit without f.txt, `bvcs restore f.txt` | `Error: 'f.txt' not found in commit NNNN.` |
| 7.4 | Confirm with `y` | `echo y | bvcs restore f.txt` | `Restored: f.txt` |
| 7.5 | Confirm with `Y` | `echo Y | bvcs restore f.txt` | `Restored: f.txt` |
| 7.6 | Reject with `n` | `echo n | bvcs restore f.txt` | `Aborted.` |
| 7.7 | Reject with any other | `echo x | bvcs restore f.txt` | `Aborted.` |
| 7.8 | Restore content | modify, restore, verify | File content reverts to committed version |

Prompt format:

```
Restore 'f.txt' from commit 0001? [y/N]:
```

---

## 8. `help`

| # | Test | Command | Expected |
|---|------|---------|----------|
| 8.1 | Show help | `bvcs help` | Lists all 8 subcommands with syntax |
| 8.2 | No args | `bvcs` | Same as `bvcs help` |

---

## 9. Error handling

| # | Test | Command | Expected |
|---|------|---------|----------|
| 9.1 | Unknown subcommand | `bvcs banana` | `Error: Unknown subcommand 'banana'.` |
| 9.2 | Outside repo | `cd /tmp && bvcs status` | `Error: Not a BVCS repository. Run 'init' first.` |
| 9.3 | Outside repo (any non-init cmd) | `bvcs add x.txt` | Same error |

---

## 10. Full end-to-end workflow

```bash
td=$(mktemp -d) && cd "$td"
bvcs init
echo '#include <stdio.h>' > main.c
echo 'int helper() {}' > util.c
echo '# Notes' > notes.md
bvcs add main.c util.c
bvcs status                     # Staged + Untracked
bvcs commit -m "Initial commit" # [0001]
echo 'int main() {}' >> main.c
bvcs status                     # Modified + Untracked
bvcs diff main.c                # Shows +int main() {}
echo y | bvcs restore main.c    # Reverts
cat main.c                      # #include <stdio.h>
bvcs log                        # One entry
echo "int main() {}" > main.c
bvcs add main.c
bvcs commit -m "Add main"       # [0002]
bvcs log                        # Two entries, most recent first
```

---

## 11. Edge cases

| # | Test | Notes |
|---|------|-------|
| 11.1 | Files in subdirectories | `mkdir -p a/b && echo x > a/b/f.txt` — add, commit, restore |
| 11.2 | Commit ID rollover | After 9999 commits, ID becomes `0001` again? Spec implies sequential; log line count determines next ID |
| 11.3 | Empty working directory after commit | `status` should still show clean |
| 11.4 | Staging with trailing newlines | `grep -qxF` handles partial matches correctly |
| 11.5 | Commit message with spaces | `bvcs commit -m "multi word message"` preserved correctly |

---

## Running all checkpoints at once

```bash
for i in $(seq 1 7); do
    echo "====== Task $i ======"
    td=$(mktemp -d) && cd "$td"
    # ... paste the checkpoint commands for task $i
done
```

Refer to the `Tasks` section of `Bash_Offline_Tutorial.pdf` for each task's exact checkpoint recipe.
