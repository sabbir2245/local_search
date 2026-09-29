<div align="center">

**January 2026 CSE 314**

**Online Assignment on Bash Scripting**

Time: 30 minutes

Subsection C1

</div>

You are a systems administrator for the VAR network at the 2026 World Cup Final. During pre-match checks, you find a security risk. Several unknown files inside `input_dir` and its subdirectories have gained **unauthorized executable permissions**.

You need to isolate these files for forensic analysis and build a timeline of the attack. Find every executable file and copy it into `output_dir`. Sort these files into subdirectories named after the **month** they were last modified. You also need to remove their execute permissions after copying them.

### Command

```bash
./online-2205XXX.sh input_dir output_dir
```

### Output

1. Find **all files with execute permissions** in `input_dir` (including subdirectories) and copy them into `output_dir/Month`.
   - If a file was last modified in July, copy it to `output_dir/Jul`.
   - Use the abbreviated month name (Jan, Feb, Mar, ...).
2. **Revoke all execute permissions** on the copied files in `output_dir`.

### Sample Input (`input_dir/`)

```
input_dir/
├── scanner.py             ← executable, last modified: May
├── readme.md              ← NOT executable — IGNORE
├── analysis/
│   ├── play_review.sh     ← executable, last modified: Jun
│   └── match_notes.txt    ← NOT executable — IGNORE
└── footage/
    └── goal_clip.dat      ← executable, last modified: Jul
```

### Sample Output (`output_dir/`)

```
output_dir/
├── May/
│   └── scanner.py         (permissions: -rw-r--r--)
├── Jun/
│   └── play_review.sh     (permissions: -rw-r--r--)
└── Jul/
    └── goal_clip.dat      (permissions: -rw-r--r--)
```
