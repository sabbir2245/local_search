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
