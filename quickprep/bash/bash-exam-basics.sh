#!/bin/bash
# ============================================================
# Bash exam basics (CSE 314) — read top to bottom, run sections.
# Every block is a pattern you will reuse in A1/A2/B2/C2/C1.
# Run: bash bash-exam-basics.sh <any_input_dir> <any_output_dir>
# ============================================================

# ---------- 0. Template every exam script starts with ----------
if [ $# -lt 2 ]; then
  echo "Usage: $0 <input_dir> <output_dir>"
  exit 1
fi
input_dir="$1"          # $1 = first argument, $2 = second. $0 = script name. $# = arg count.
output_dir="$2"
[ -d "$input_dir" ] || { echo "Error: '$input_dir' not a directory."; exit 1; }
mkdir -p "$output_dir"  # -p = no error if exists, creates parents.

# ---------- 1. Variables & quoting ----------
name="world"                    # NO spaces around = . Always quote expansions: "$name"
echo "hello $name"              # double quotes: expand. 'hello $name' would print literally $name.
echo "args: $1 $2, count: $#, all: $@"
base=$(basename "/a/b/file.txt")  # command substitution: $(...) preferred over `...`
echo "base=$base"               # -> file.txt
noext="${base%.txt}"            # strip suffix: ${var%pattern}. prefix strip: ${var#pattern}
echo "noext=$noext"

# ---------- 2. Arithmetic ----------
a=5; b=3
sum=$((a + b))                  # $((...)) for math. Also: $((a++)) $((a % b))
echo "sum=$sum"
count=$(grep -c "x" "$0")       # grep -c counts MATCHING LINES (not occurrences)
echo "lines with x in this file: $count"
n=$(wc -l < "$0")               # line count of a file (note < to get bare number)
echo "total lines: $n"

# ---------- 3. Tests: files, strings, numbers ----------
# [ ... ] or [[ ... ]]. Memorize: -f file, -d dir, -s nonempty, -x executable, -e exists,
# -z empty string, -n nonempty string, =/!= strings, -eq -ne -lt -le -gt -ge numbers.
[ -f "$0" ] && echo "I am a file"
[ -d "$output_dir" ] && echo "output dir exists"
[ -s "$0" ] && echo "nonempty"
[ -x "$0" ] && echo "executable"
[ "$a" -gt "$b" ] && echo "a bigger"
[ "$name" = "world" ] && echo "name matches"

# ---------- 4. If / case ----------
if [ "$a" -gt "$b" ]; then
  echo "a wins"
elif [ "$a" -eq "$b" ]; then
  echo "tie"
else
  echo "b wins"
fi
case "$1" in
  *.log) echo "log file";;      # pattern match, ;; ends branch
  *.txt) echo "text file";;
  *)     echo "other";;
esac

# ---------- 5. Loops (the exam workhorses) ----------
# 5a. C-style + seq + glob loops
for i in 1 2 3; do echo "i=$i"; done
for i in $(seq 0 2); do echo "seq=$i"; done
for f in "$input_dir"/*; do     # direct children only (B2 pattern). Quote dir, not the *.
  [ -e "$f" ] || continue       # skip when glob matches nothing
  echo "child: $f"
done
# 5b. find loops — ALWAYS this exact shape in exams (handles spaces):
while IFS= read -r file; do
  echo "found: $file"
done < <(find "$input_dir" -type f 2>/dev/null)            # all files, recursive (A1 pattern)
# find variants you will need:
#   find "$input_dir" -type f -name "*.log"                # A2: extension filter
#   find "$input_dir" -type f -name "FINAL_*"              # C2: prefix filter
#   find "$input_dir" -type f -executable                  # C1: executable only
#   find ... | sort                                        # sorted paths: ... < <(find ... | sort)

# ---------- 6. Reading & counting inside files ----------
f="$0"
winter=$(grep -ic "echo" "$f")        # -i case-insensitive, -c count lines
echo "echo-lines=$winter"
size=$(stat -c%s "$f")                # file size in BYTES (B2 pattern)
echo "my size=$size"
month=$(date -r "$f" +%b)             # abbreviated month: May Jun Jul (C1 pattern)
echo "my month=$month"

# ---------- 7. Copy / move / rename ----------
cp "$f" "$output_dir/"                               # copy file
cp -- "$f" "$output_dir/renamed.sh"                  # -- protects names starting with -
mv -- "$output_dir/renamed.sh" "$output_dir/kept.sh" # rename = move
mkdir -p "$output_dir/Small" "$output_dir/Medium" "$output_dir/Large"  # B2 dirs
# rename extension: ./sample.sh .dat .txt  (quiz Q2 pattern)
# for g in *".dat"; do [ -e "$g" ] || continue; mv -- "$g" "${g%.dat}.txt"; done

# ---------- 8. Size buckets (B2), month buckets (C1), keyword buckets (A1) ----------
if [ "$size" -lt 50 ]; then echo "Small";
elif [ "$size" -le 99 ]; then echo "Medium";
else echo "Large"; fi
mkdir -p "$output_dir/$month" && cp "$f" "$output_dir/$month/" && chmod -x "$output_dir/$month/$(basename "$f")" 2>/dev/null
# chmod -x removes execute permission (C1). chmod +x adds it.

# ---------- 9. Sorting (exam ordering rules) ----------
# sort paths alphabetically:        find ... | sort
# sort "name:count" by name:        sort -t: -k1,1 file
# sort "size:name" numeric desc:    sort -t: -k1,1nr file   (B2 sizes.txt)
# sort by file size, smallest first: ls -Sr dir            (A1 numbering)
# unique filenames assumed; ties won't occur per statements.

# ---------- 10. Redirection, pipes, output files ----------
echo "hello" > "$output_dir/demo.txt"    # > overwrite, >> append, < input
echo "again" >> "$output_dir/demo.txt"
: > "$output_dir/empty.txt"              # : > f  = truncate/create empty file
cat "$output_dir/demo.txt"
history 2>/dev/null | grep -w grep > "$output_dir/grep.txt"  # quiz Q3b pattern
sed -n '15,22p' "$0" > /dev/null         # print lines 15-22 (quiz Q3a)
echo "total-output-lines: $(wc -l < "$output_dir/demo.txt")"

# ---------- 11. Functions ----------
greet() { echo "hi $1"; }   # $1 inside function = first function arg (shadows script $1)
greet "exam"

# ---------- 12. Arrays (rare, but cheap marks) ----------
arr=("a" "b c" "d")
echo "first=${arr[0]}, count=${#arr[@]}, all=${arr[@]}"
for x in "${arr[@]}"; do echo "el=$x"; done   # quotes keep "b c" together

echo "Done. Basics complete."
