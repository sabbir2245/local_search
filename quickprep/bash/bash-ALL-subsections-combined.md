# Bash — All Subsections Combined
Sources (in order): `bash-a1-c1-combined.md`, `bash-a2-problem-solution.md`, `bash-b2-problem-solution.md`, `bash-c2-problem-solution.md`, `bash-similar-problems.md`, `bash-similar-problems-solve.md`


---

<!-- ===== bash-a1-c1-combined.md ===== -->

# A1 + C1 Combined

## A1 — Raven Messages (Stark/Targaryen/Royal) — from Downloads/A1.zip PDF

```text
January 2026 CSE 314: Online 1 (A1)


                                       Time: 25 minutes

   During the battle for Westeros, raven messages from different regions have become mixed
throughout the archive. Each message may contain the following keywords:

winter
dragon
throne



Command
./online-2205XXX.sh <input_dir> <output_dir>



Requirements
1. Create output_dir if it does not already exist. Then create the following three subdirectories
   inside it:

   output_dir/
   |-- Stark/
   |-- Targaryen/
   |-- Royal/


2. Search through input_dir and all of its subdirectories.

3. For every file, calculate the following values using case-insensitive keyword matching:

  • winter_count: number of lines containing winter
  • dragon_count: number of lines containing dragon
  • throne_count: number of lines containing throne
  • total_score: sum of the three counts

4. Append total_score to the end of the file’s complete original filename, including after its
   extension. For example, if warning.msg has a total score of 4, its new name will be:

   warning.msg_4


5. Move every file according to the keyword with the highest count:

  • Move it to output_dir/Stark/ if winter_count is the highest.
  • Move it to output_dir/Targaryen/ if dragon_count is the highest.
  • Move it to output_dir/Royal/ if throne_count is the highest.

                                               1
6. After all files have been moved to their appropriate directories, add a numerical prefix to
   each file according to its position in the file-size order. Numbering must begin independently
   from 0 inside each category directory. For example:

   0_warning.msg_4
   1_prophecy.txt_6
   2_battle.log_9



Assumptions
You may assume that:

• All input files are regular readable files.
• Every file has exactly one keyword whose count is strictly greater than the other two keyword
  counts. Therefore, classification ties will not occur.
• No line in any input file contains a keyword more than once.
• No two files moved in the same category directory have the same file size.
• Every input file has a unique filename, even when the files are in different subdirectories.
• input_dir and output_dir refer to different directory trees.


Sample Input
input_dir/
|-- north/
|   ‘-- warning.msg
|-- east/
|   |-- prophecy.log
|   ‘-- dragon_report.data
‘-- capital/
    ‘-- royal_order.txt



Sample Output
Assuming prophecy.log_4 is smaller than dragon_report.data_3, the output is:

output_dir/
|-- Stark/
|   ‘-- 0_warning.msg_3
|-- Targaryen/
|   |-- 0_prophecy.log_4
|   ‘-- 1_dragon_report.data_3
‘-- Royal/
    ‘-- 0_royal_order.txt_4




                                                2
```

### A1 solution (`bash_offline/a1solve.sh`)

```bash
#!/usr/bin/bash

# ============================================
# A1 — Raven Message Sorter
# ============================================
# Usage: ./a1solve.sh <input_dir> <output_dir>
#
# Scans all files in input_dir, counts keywords
# (winter / dragon / throne), and sorts each
# file into Stark, Targaryen, or Royal based
# on which keyword appears most often.
# ============================================

# --- CHECK COMMAND-LINE ARGUMENTS ---
if [ $# -lt 2 ]; then
    echo "Usage: $0 <input_dir> <output_dir>"
    exit 1
fi

input_dir="$1"
output_dir="$2"

if [ ! -d "$input_dir" ]; then
    echo "Error: '$input_dir' is not a valid directory."
    exit 1
fi

# --- CREATE OUTPUT FOLDERS ---
mkdir -p "$output_dir/Stark"
mkdir -p "$output_dir/Targaryen"
mkdir -p "$output_dir/Royal"

# --- PROCESS EVERY FILE IN THE INPUT ---
# Find all regular files recursively
while IFS= read -r file; do
    filename=$(basename "$file")

    winter_count=$(grep -ic "winter" "$file" 2>/dev/null)
    dragon_count=$(grep -ic "dragon" "$file" 2>/dev/null)
    throne_count=$(grep -ic "throne" "$file" 2>/dev/null)

    total_score=$(( winter_count + dragon_count + throne_count ))

    new_name="${filename}_${total_score}"

    if [ "$winter_count" -gt "$dragon_count" ] && [ "$winter_count" -gt "$throne_count" ]; then
        cp "$file" "$output_dir/Stark/$new_name"
    elif [ "$dragon_count" -gt "$winter_count" ] && [ "$dragon_count" -gt "$throne_count" ]; then
        cp "$file" "$output_dir/Targaryen/$new_name"
    else
        cp "$file" "$output_dir/Royal/$new_name"
    fi
done < <(find "$input_dir" -type f 2>/dev/null)

# --- RENAME FILES WITH SIZE ORDER PREFIX ---
for category in Stark Targaryen Royal; do
    category_dir="$output_dir/$category"
    if [ ! -d "$category_dir" ]; then
        continue
    fi
    counter=0
    while IFS= read -r filename; do
        mv "$category_dir/$filename" "$category_dir/${counter}_$filename"
        counter=$((counter + 1))
    done < <(ls -Sr "$category_dir" 2>/dev/null)
done

echo "Done. Files sorted into $output_dir/{Stark,Targaryen,Royal}."
```

---

## C1 — Executable-File Forensics by Month — from `bash_offline/C1/C1/problem.md`

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


### C1 solution (`bash_offline/c1solve.sh`)

```bash
#!/usr/bin/bash

# ============================================
# C1 — Executable File Isolator
# ============================================
# Usage: ./c1solve.sh <input_dir> <output_dir>
#
# Finds every executable file inside input_dir,
# copies it into output_dir/<Month>/, and then
# removes the execute permission on the copy.
# ============================================

# --- CHECK COMMAND-LINE ARGUMENTS ---
if [ $# -lt 2 ]; then
    echo "Usage: $0 <input_dir> <output_dir>"
    exit 1
fi

input_dir="$1"
output_dir="$2"

if [ ! -d "$input_dir" ]; then
    echo "Error: '$input_dir' is not a valid directory."
    exit 1
fi

# --- CREATE OUTPUT FOLDER ---
mkdir -p "$output_dir"

# --- FIND EXECUTABLE FILES ---
while IFS= read -r file; do
    month=$(date -r "$file" +%b)

    month_dir="$output_dir/$month"
    mkdir -p "$month_dir"

    cp "$file" "$month_dir/"

    copied_file="$month_dir/$(basename "$file")"
    chmod -x "$copied_file"
done < <(find "$input_dir" -type f -executable 2>/dev/null)

echo "Done. Executable files copied to $output_dir/<Month>/ with execute permission removed."
```


---

<!-- ===== bash-a2-problem-solution.md ===== -->

# A2 — Commentary Archive (.log collect)

## Problem statement (from Downloads/A2.zip `A2/A2.md`)

---
course: January 2026 CSE 314
assignment: Online Assignment on Bash Scripting
time: 30 minutes
subsection: A2
---

You are preparing the commentary archive for the World Cup final between Argentina and Spain. Commentary logs from different parts of the match are stored inside `input_dir` and its subdirectories.

Your script must collect the `.log` files in a predictable order and create one combined match report.

### Command

```bash
./online-2205XXX.sh input_dir output_dir
```

### Output

1. Create `output_dir` if it does not exist.

2. Search recursively for regular files whose names end with `.log`. Ignore all other files.

3. Sort the selected files alphabetically by their complete paths.

4. Create `output_dir/match_report.txt`. Process the selected files in the alphabetically sorted path order from step 3. For every file, add:

   ```text
   ===== original_filename =====
   ```

   Then append the complete contents of that file. Add one blank line after each file's contents.

5. Create `output_dir/file_list.txt`. Add one line for every selected file:

   ```text
   original_filename:line_count
   ```

   Sort these lines alphabetically by original filename.

6. Create `output_dir/total_lines.txt` containing only the total number of lines across all selected files.

\newpage

### Sample Input (`input_dir/`)

```text
input_dir/
├── first_half/
│   ├── opening.log
│   └── notes.txt
├── second_half/
│   ├── goals.log
│   └── cards.log
└── extra_time/
    └── final_whistle.log
```

### Sample Output (`output_dir/`)

```text
output_dir/
├── match_report.txt
├── file_list.txt
└── total_lines.txt
```

Contents of `file_list.txt`:

```text
cards.log:2
final_whistle.log:1
goals.log:3
opening.log:2
```

Contents of `total_lines.txt`:

```text
8
```

### Assumptions

- Every selected file has a unique filename.
- At least one `.log` file exists.


---

## Solution (`online-2205XXX.sh`, verified against expected_output)

```bash
#!/bin/bash
# A2 solution: collect .log files in path-sorted order into one match report
if [ $# -lt 2 ]; then echo "Usage: $0 <input_dir> <output_dir>"; exit 1; fi
input_dir="$1"; output_dir="$2"
mkdir -p "$output_dir"
mapfile -t files < <(find "$input_dir" -type f -name "*.log" | sort)
: > "$output_dir/match_report.txt"
: > /tmp/a2_filelist_tmp.txt
total=0
for f in "${files[@]}"; do
  base=$(basename "$f")
  echo "===== $base =====" >> "$output_dir/match_report.txt"
  cat "$f" >> "$output_dir/match_report.txt"
  echo "" >> "$output_dir/match_report.txt"
  n=$(wc -l < "$f")
  echo "$base:$n" >> /tmp/a2_filelist_tmp.txt
  total=$((total + n))
done
sort -t: -k1,1 /tmp/a2_filelist_tmp.txt > "$output_dir/file_list.txt"
echo "$total" > "$output_dir/total_lines.txt"
rm -f /tmp/a2_filelist_tmp.txt
```


---

<!-- ===== bash-b2-problem-solution.md ===== -->

# B2 — Broadcast Package by Size

## Problem statement (from Downloads/B2.zip `B2/B2.md`)

---
course: January 2026 CSE 314
assignment: Online Assignment on Bash Scripting
time: 30 minutes
subsection: B2
---

You are organizing files for the World Cup final broadcast package. The files directly inside `input_dir` must be separated according to their sizes so that the production team can choose an appropriate transfer method.

### Command

```bash
./online-2205XXX.sh input_dir output_dir
```

### Output

1. Create the following directories:

   ```text
   output_dir/
   ├── Small/
   ├── Medium/
   └── Large/
   ```

2. Consider only non-empty regular files directly inside `input_dir`. Do not search subdirectories.

3. Calculate each file's size in bytes.

4. Copy each file according to its size:

   - `Small/` if its size is less than 50 bytes
   - `Medium/` if its size is from 50 through 99 bytes
   - `Large/` if its size is 100 bytes or more

5. Keep every copied file's original filename.

6. Create `output_dir/sizes.txt`. Add one line for every copied file:

   ```text
   size_in_bytes:original_filename
   ```

   Sort the lines numerically by size in descending order.

\newpage

### Sample Input (`input_dir/`)

```text
input_dir/
├── lineup.txt       (32 bytes)
├── tactics.dat      (76 bytes)
├── full_report.log  (138 bytes)
├── schedule.txt     (47 bytes)
└── empty.tmp        (empty — IGNORE)
```

### Sample Output (`output_dir/`)

```text
output_dir/
├── Small/
│   ├── lineup.txt
│   └── schedule.txt
├── Medium/
│   └── tactics.dat
├── Large/
│   └── full_report.log
└── sizes.txt
```

Contents of `sizes.txt`:

```text
138:full_report.log
76:tactics.dat
47:schedule.txt
32:lineup.txt
```

### Assumptions

- All files directly inside `input_dir` have unique filenames.
- No two processed files have the same size.


---

## Solution (`online-2205XXX.sh`, verified against expected_output)

```bash
#!/bin/bash
# B2 solution: separate files directly inside input_dir by size
if [ $# -lt 2 ]; then echo "Usage: $0 <input_dir> <output_dir>"; exit 1; fi
input_dir="$1"; output_dir="$2"
mkdir -p "$output_dir/Small" "$output_dir/Medium" "$output_dir/Large"
: > /tmp/b2_sizes_tmp.txt
for f in "$input_dir"/*; do
  [ -f "$f" ] || continue
  [ -s "$f" ] || continue
  size=$(stat -c%s "$f")
  base=$(basename "$f")
  if [ "$size" -lt 50 ]; then cp "$f" "$output_dir/Small/"
  elif [ "$size" -le 99 ]; then cp "$f" "$output_dir/Medium/"
  else cp "$f" "$output_dir/Large/"
  fi
  echo "$size:$base" >> /tmp/b2_sizes_tmp.txt
done
sort -t: -k1,1nr /tmp/b2_sizes_tmp.txt > "$output_dir/sizes.txt"
rm -f /tmp/b2_sizes_tmp.txt
```


---

<!-- ===== bash-c2-problem-solution.md ===== -->

# C2 — FINAL_* Delivery Folder

## Problem statement (from Downloads/C2.zip `C2/C2.md`)

---
course: January 2026 CSE 314
assignment: Online Assignment on Bash Scripting
time: 30 minutes
subsection: C2
---

You are preparing the final delivery folder for the World Cup final between Argentina and Spain. Approved files are identified by filenames beginning with `FINAL_`, but they are scattered throughout `input_dir`.

Your script must collect the approved files without losing their original directory structure.

### Command

```bash
./online-2205XXX.sh input_dir output_dir
```

### Output

1. Create:

   ```text
   output_dir/package/
   ```

2. Search recursively for regular files whose filenames begin with `FINAL_`. Matching is case-sensitive.

3. Copy every selected file into `output_dir/package/` while preserving its path relative to `input_dir`.

   For example:

   ```text
   input_dir/var/video/FINAL_replay.dat
   ```

   must become:

   ```text
   output_dir/package/var/video/FINAL_replay.dat
   ```

4. Create `output_dir/manifest.txt` containing the relative path of every copied file.

5. Sort `manifest.txt` alphabetically.

6. Create `output_dir/count.txt` containing only the total number of copied files.

\newpage

### Sample Input (`input_dir/`)

```text
input_dir/
├── broadcast/
│   ├── FINAL_scoreboard.txt
│   └── draft_notes.txt
├── var/
│   └── video/
│       ├── FINAL_replay.dat
│       └── test_clip.dat
└── reports/
    └── FINAL_summary.log
```

### Sample Output (`output_dir/`)

```text
output_dir/
├── package/
│   ├── broadcast/
│   │   └── FINAL_scoreboard.txt
│   ├── reports/
│   │   └── FINAL_summary.log
│   └── var/
│       └── video/
│           └── FINAL_replay.dat
├── manifest.txt
└── count.txt
```

Contents of `manifest.txt`:

```text
broadcast/FINAL_scoreboard.txt
reports/FINAL_summary.log
var/video/FINAL_replay.dat
```

Contents of `count.txt`:

```text
3
```


---

## Solution (`online-2205XXX.sh`, verified against expected_output)

```bash
#!/bin/bash
# C2 solution: collect FINAL_* files preserving relative structure
if [ $# -lt 2 ]; then echo "Usage: $0 <input_dir> <output_dir>"; exit 1; fi
input_dir="$1"; output_dir="$2"
mkdir -p "$output_dir/package"
: > /tmp/c2_manifest_tmp.txt
while IFS= read -r f; do
  rel=${f#"${input_dir%/}/"}
  mkdir -p "$output_dir/package/$(dirname "$rel")"
  cp "$f" "$output_dir/package/$rel"
  echo "$rel" >> /tmp/c2_manifest_tmp.txt
done < <(find "$input_dir" -type f -name "FINAL_*")
sort /tmp/c2_manifest_tmp.txt > "$output_dir/manifest.txt"
wc -l < "$output_dir/manifest.txt" | tr -d ' ' > "$output_dir/count.txt"
rm -f /tmp/c2_manifest_tmp.txt
```


---

<!-- ===== bash-similar-problems.md ===== -->

# 📝 Similar Exam Problems

## Problem 1: Text File Collector by Line Count

**Goal:** Write a script that finds every **text file** (regardless of extension) inside `<input_dir>`, sorts them by line count (ascending), and copies them into `<output_dir>` with a numerical prefix.

**Usage:** `./p1solve.sh <input_dir> <output_dir>`

**Requirements:**
- Detect text files by **content** (MIME type), not by filename extension.
- Sort by line count from fewest to most.
- Name the copies `0_filename`, `1_filename`, `2_filename`, etc. (starting at 0).
- On a tie in line count, preserve the original `find` order.

**Sample Input (`input/`):**

```
input/
├── notes.txt        (3 lines)
├── data.csv         (5 lines)
├── script.py        (2 lines)
└── image.png        (binary, skip)
```

**Sample Output (`output/`):**

```
output/
├── 0_script.py
├── 1_notes.txt
└── 2_data.csv
```

---

## Problem 2: Media File Sorter by Size Tier

**Goal:** Write a script that finds every **media file** (`.mp3`, `.flac`, `.mp4`, `.mkv`) inside `<input_dir>`, categorizes each by file size, and copies it into a corresponding subfolder.

**Usage:** `./p2solve.sh <input_dir> <output_dir>`

**Requirements:**
- Match extensions case-insensitively (`.MP3`, `.Flac`, etc.).
- Size tiers:
  - `Small` — size ≤ 1 MB (1,048,576 bytes)
  - `Medium` — size ≤ 100 MB (104,857,600 bytes)
  - `Large` — everything larger
- Preserve the original filename.
- Remove write permission (`chmod -w`) from every copied file.

**Sample Input (`input/`):**

```
input/
├── song.mp3        (500 KB)
├── video.mp4       (50 MB)
├── movie.mkv       (2 GB)
└── notes.txt       (skip — wrong extension)
```

**Sample Output (`output/`):**

```
output/
├── Small/
│   └── song.mp3          (chmod -w applied)
├── Medium/
│   └── video.mp4         (chmod -w applied)
└── Large/
    └── movie.mkv         (chmod -w applied)
```

---

## Problem 3: Hidden File Backup by Year

**Goal:** Write a script that finds every **hidden file** (name starts with `.`) inside `<input_dir>`, groups them by their **modification year**, and copies them into `<output_dir>/<year>/` while preserving the relative directory structure.

**Usage:** `./p3solve.sh <input_dir> <output_dir>`

**Requirements:**
- A hidden file is any file whose basename begins with `.`.
- Preserve the path relative to `input_dir` inside the year subfolder.
- Extract the year from the file's modification timestamp (format: `+%Y`).
- Do **not** copy hidden directories — only regular files.

**Sample Input (`input/`):**

```
input/
├── .config/
│   ├── settings.json      (mod 2024)
│   └── .secret.txt        (mod 2025)
├── .bashrc                (mod 2024)
├── docs/
│   └── .notes             (mod 2025)
└── README                 (not hidden — skip)
```

**Sample Output (`output/`):**

```
output/
├── 2024/
│   ├── .config/settings.json
│   └── .bashrc
└── 2025/
    ├── .config/.secret.txt
    └── docs/.notes
```

---

## Problem 4: Log File Anomaly Detector

**Goal:** Write a script that scans every **text file** inside `<input_dir>`, counts occurrences of the keywords `ERROR`, `WARN`, and `INFO` (case-insensitive, whole-word), classifies the file by the most frequent keyword, and copies it with a **score suffix** inserted before the extension.

**Usage:** `./p4solve.sh <input_dir> <output_dir>`

**Requirements:**
- Count **total occurrences** (not lines) using `grep -oiw`.
- Classification: most frequent keyword wins.
  - Ties → `UNKNOWN` category.
- Score = sum of all three keyword counts.
- Output name: `filename_SCORE.ext` (e.g., `server_ERROR_12.log`).
  - Files without an extension get `filename_SCORE`.

**Sample Input (`input/`):**

```
input/
├── server.log
│   (ERROR × 5, WARN × 2, INFO × 1)
├── app.log
│   (WARN × 3, ERROR × 3, INFO × 3)     ← tie
└── system.log
│   (INFO × 8, ERROR × 1, WARN × 0)
```

**Sample Output (`output/`):**

```
output/
├── ERROR/
│   └── server_ERROR_8.log
├── UNKNOWN/
│   └── app_UNKNOWN_9.log
└── INFO/
    └── system_INFO_9.log
```


---

<!-- ===== bash-similar-problems-solve.md ===== -->

# Solutions — Similar Exam Problems

---

## Problem 1 — Text File Collector by Line Count

**File:** `p1solve.sh`

```bash
#!/usr/bin/bash

if [ $# -lt 2 ]; then
    echo "Usage: $0 <input_dir> <output_dir>"
    exit 1
fi

input_dir="$1"
output_dir="$2"

if [ ! -d "$input_dir" ]; then
    echo "Error: '$input_dir' is not a valid directory."
    exit 1
fi

mkdir -p "$output_dir"

records=()

while IFS= read -r -d '' file; do
    mime=$(file -b --mime-type "$file" 2>/dev/null)
    if [[ "$mime" == text/* ]]; then
        lines=$(wc -l < "$file")
        records+=("$lines"$'\t'"$file")
    fi
done < <(find "$input_dir" -type f -print0 2>/dev/null)

count=0

while IFS= read -r -d '' entry; do
    file="${entry#*$'\t'}"
    filename=$(basename "$file")
    cp "$file" "$output_dir/${count}_$filename"
    ((count++))
done < <(
    printf '%s\0' "${records[@]}" |
        sort -z -n -t $'\t' -k1,1
)

echo "Done. Text files copied to $output_dir with line-count prefixes."
```

---

## Problem 2 — Media File Sorter by Size Tier

**File:** `p2solve.sh`

```bash
#!/usr/bin/bash

if [ $# -lt 2 ]; then
    echo "Usage: $0 <input_dir> <output_dir>"
    exit 1
fi

input_dir="$1"
output_dir="$2"

if [ ! -d "$input_dir" ]; then
    echo "Error: '$input_dir' is not a valid directory."
    exit 1
fi

mkdir -p "$output_dir/Small" "$output_dir/Medium" "$output_dir/Large"

while IFS= read -r -d '' file; do
    filename=$(basename "$file")
    size=$(stat --format=%s "$file" 2>/dev/null)

    if (( size <= 1048576 )); then
        category="Small"
    elif (( size <= 104857600 )); then
        category="Medium"
    else
        category="Large"
    fi

    cp "$file" "$output_dir/$category/$filename"
    chmod -w "$output_dir/$category/$filename"
done < <(
    find "$input_dir" -type f \
        \( -iname '*.mp3' -o -iname '*.flac' \
           -o -iname '*.mp4' -o -iname '*.mkv' \) \
        -print0 2>/dev/null
)

echo "Done. Media files sorted into $output_dir/{Small,Medium,Large}."
```

---

## Problem 3 — Hidden File Backup by Year

**File:** `p3solve.sh`

```bash
#!/usr/bin/bash

if [ $# -lt 2 ]; then
    echo "Usage: $0 <input_dir> <output_dir>"
    exit 1
fi

input_dir="${1%/}"
output_dir="${2%/}"

if [ ! -d "$input_dir" ]; then
    echo "Error: '$input_dir' is not a valid directory."
    exit 1
fi

mkdir -p "$output_dir"

while IFS= read -r -d '' file; do
    filename=$(basename "$file")
    [[ "$filename" != .* ]] && continue

    year=$(date -r "$file" +%Y)
    relative="${file#"$input_dir"/}"
    destination="$output_dir/$year/$relative"

    mkdir -p "$(dirname "$destination")"
    cp "$file" "$destination"
done < <(find "$input_dir" -type f -print0 2>/dev/null)

echo "Done. Hidden files copied to $output_dir/<year>/ with relative paths."
```

---

## Problem 4 — Log File Anomaly Detector

**File:** `p4solve.sh`

```bash
#!/usr/bin/bash

if [ $# -lt 2 ]; then
    echo "Usage: $0 <input_dir> <output_dir>"
    exit 1
fi

input_dir="$1"
output_dir="$2"

if [ ! -d "$input_dir" ]; then
    echo "Error: '$input_dir' is not a valid directory."
    exit 1
fi

mkdir -p "$output_dir/ERROR" "$output_dir/WARN" "$output_dir/INFO" "$output_dir/UNKNOWN"

while IFS= read -r -d '' file; do
    mime=$(file -b --mime-type "$file" 2>/dev/null)
    [[ "$mime" != text/* ]] && continue

    filename=$(basename "$file")

    err=$(grep -oiw -- 'ERROR' "$file" 2>/dev/null | wc -l)
    warn=$(grep -oiw -- 'WARN' "$file" 2>/dev/null | wc -l)
    info=$(grep -oiw -- 'INFO' "$file" 2>/dev/null | wc -l)

    total=$((err + warn + info))

    if (( err > warn && err > info )); then
        category="ERROR"
    elif (( warn > err && warn > info )); then
        category="WARN"
    elif (( info > err && info > warn )); then
        category="INFO"
    else
        category="UNKNOWN"
    fi

    stem="${filename%.*}"
    ext="${filename##*.}"

    if [ "$stem" = "$ext" ]; then
        new_name="${stem}_${category}_${total}"
    else
        new_name="${stem}_${category}_${total}.${ext}"
    fi

    cp "$file" "$output_dir/$category/$new_name"
done < <(find "$input_dir" -type f -print0 2>/dev/null)

echo "Done. Files classified into $output_dir/{ERROR,WARN,INFO,UNKNOWN}."
```
