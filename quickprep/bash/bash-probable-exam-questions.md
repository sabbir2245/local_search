# Probable Bash Exam Questions + Solutions

Inspired by `../quiz-2022-solved.md` (Q2 extension-rename, Q3 line-range/history, B-Q1 `exec`, B-Q2 quoting)
and the A1/A2/B2/C2/C1 offline patterns. Each follows the exam command shape:
`./online-2205XXX.sh input_dir output_dir`. All solutions tested mentally against the stated edge cases.

---

## P1 — Extension swap with count report (quiz Q2 style, probable)

**Problem.** Take two extensions as args: `./online-2205XXX.sh .dat .txt input_dir output_dir`.
Copy every `*.dat` under `input_dir` (recursive) into `output_dir`, renamed to `*.txt`,
preserving relative paths. Write `output_dir/renamed.txt` listing `oldpath -> newpath`
sorted alphabetically, and `output_dir/count.txt` with the total.

**Solution.**
```bash
#!/bin/bash
if [ $# -ne 4 ]; then echo "Usage: $0 <old> <new> <input_dir> <output_dir>"; exit 1; fi
old="$1"; new="$2"; input_dir="$3"; output_dir="$4"
mkdir -p "$output_dir"
: > /tmp/p1_tmp.txt
while IFS= read -r f; do
  rel=${f#"${input_dir%/}/"}
  newrel="${rel%$old}$new"
  mkdir -p "$output_dir/$(dirname "$newrel")"
  cp -- "$f" "$output_dir/$newrel"
  echo "$rel -> $newrel" >> /tmp/p1_tmp.txt
done < <(find "$input_dir" -type f -name "*$old" | sort)
sort /tmp/p1_tmp.txt > "$output_dir/renamed.txt"
wc -l < "$output_dir/renamed.txt" | tr -d ' ' > "$output_dir/count.txt"
rm -f /tmp/p1_tmp.txt
```
Traps: quote `"$f"`; `--` for dash-names; `${var%pattern}` strips only the suffix.

## P2 — Line-range extract + history audit (quiz Q3 style, probable)

**Problem.** (a) Given a log file, print lines 10–20 inclusive.
(b) From `input_dir` (recursive), find `.sh` files containing the word `grep`,
copy them to `output_dir/`, and write `output_dir/hits.txt` as `filename:count`
(count = matching lines, `grep -c`), sorted by filename.

**Solution.**
```bash
# (a)
sed -n '10,20p' app.log
# (b)
mkdir -p "$output_dir"
: > /tmp/p2_tmp.txt
while IFS= read -r f; do
  c=$(grep -c "grep" "$f")
  [ "$c" -gt 0 ] || continue
  cp -- "$f" "$output_dir/"
  echo "$(basename "$f"):$c" >> /tmp/p2_tmp.txt
done < <(find "$input_dir" -type f -name "*.sh" | sort)
sort -t: -k1,1 /tmp/p2_tmp.txt > "$output_dir/hits.txt"
rm -f /tmp/p2_tmp.txt
```

## P3 — Keyword classify + size-order numbering (A1 style, highly probable)

**Problem.** Like A1 but with keywords `error/warn/info` (case-insensitive).
For each file: `*_count` = lines containing the keyword, `total` = sum,
rename to `orig_total`, move to `output_dir/ERROR|WARN|INFO/` by highest count,
then prefix `0_ 1_ ...` by file-size order in each dir.

**Solution.**
```bash
#!/bin/bash
input_dir="$1"; output_dir="$2"
mkdir -p "$output_dir"/ERROR "$output_dir"/WARN "$output_dir"/INFO
while IFS= read -r f; do
  base=$(basename "$f")
  e=$(grep -ic "error" "$f"); w=$(grep -ic "warn" "$f"); i=$(grep -ic "info" "$f")
  total=$((e + w + i)); name="${base}_${total}"
  if [ "$e" -gt "$w" ] && [ "$e" -gt "$i" ]; then cp -- "$f" "$output_dir/ERROR/$name"
  elif [ "$w" -gt "$e" ] && [ "$w" -gt "$i" ]; then cp -- "$f" "$output_dir/WARN/$name"
  else cp -- "$f" "$output_dir/INFO/$name"; fi
done < <(find "$input_dir" -type f)
for d in ERROR WARN INFO; do
  n=0
  while IFS= read -r g; do mv -- "$output_dir/$d/$g" "$output_dir/$d/${n}_$g"; n=$((n+1)); done < <(ls -Sr "$output_dir/$d")
done
```
Traps: `grep -ic` (not `grep -c` — case matters); `ls -Sr` = smallest first.

## P4 — Collect + combined report (A2 style, highly probable)

**Problem.** Collect `*.csv` files in path-sorted order into `output_dir/all.csv`
with `===== filename =====` headers + blank line after each; write
`output_dir/stats.txt` (`filename:lines`, sorted by name) and `output_dir/total.txt`.

**Solution.**
```bash
#!/bin/bash
input_dir="$1"; output_dir="$2"; mkdir -p "$output_dir"
: > "$output_dir/all.csv"; : > /tmp/p4_tmp.txt; total=0
while IFS= read -r f; do
  b=$(basename "$f")
  echo "===== $b =====" >> "$output_dir/all.csv"
  cat -- "$f" >> "$output_dir/all.csv"; echo "" >> "$output_dir/all.csv"
  n=$(wc -l < "$f"); echo "$b:$n" >> /tmp/p4_tmp.txt; total=$((total+n))
done < <(find "$input_dir" -type f -name "*.csv" | sort)
sort -t: -k1,1 /tmp/p4_tmp.txt > "$output_dir/stats.txt"
echo "$total" > "$output_dir/total.txt"; rm -f /tmp/p4_tmp.txt
```

## P5 — Size buckets + numeric-desc report (B2 style, highly probable)

**Problem.** Direct children only, skip empties: `<1K` → `Small/`, `1K–10K` → `Medium/`,
`>10K` → `Large/` (bytes: 1024/10240). Report `output_dir/sizes.txt` as
`bytes:filename` sorted numerically descending.

**Solution.**
```bash
#!/bin/bash
input_dir="$1"; output_dir="$2"; mkdir -p "$output_dir"/Small "$output_dir"/Medium "$output_dir"/Large
: > /tmp/p5_tmp.txt
for f in "$input_dir"/*; do
  [ -f "$f" ] || continue; [ -s "$f" ] || continue
  s=$(stat -c%s "$f"); b=$(basename "$f")
  if [ "$s" -lt 1024 ]; then cp -- "$f" "$output_dir/Small/"
  elif [ "$s" -le 10240 ]; then cp -- "$f" "$output_dir/Medium/"
  else cp -- "$f" "$output_dir/Large/"; fi
  echo "$s:$b" >> /tmp/p5_tmp.txt
done
sort -t: -k1,1nr /tmp/p5_tmp.txt > "$output_dir/sizes.txt"; rm -f /tmp/p5_tmp.txt
```
Traps: no `find` (direct children only); skip empties with `-s`; `sort -nr` descending.

## P6 — Prefix collect preserving tree (C2 style, probable)

**Problem.** Copy `PASS_*` files (recursive, case-sensitive) into
`output_dir/pkg/` preserving relative paths; write sorted `manifest.txt` + `count.txt`.

**Solution.**
```bash
#!/bin/bash
input_dir="$1"; output_dir="$2"; mkdir -p "$output_dir/pkg"
: > /tmp/p6_tmp.txt
while IFS= read -r f; do
  rel=${f#"${input_dir%/}/"}
  mkdir -p "$output_dir/pkg/$(dirname "$rel")"
  cp -- "$f" "$output_dir/pkg/$rel"
  echo "$rel" >> /tmp/p6_tmp.txt
done < <(find "$input_dir" -type f -name "PASS_*")
sort /tmp/p6_tmp.txt > "$output_dir/manifest.txt"
wc -l < "$output_dir/manifest.txt" | tr -d ' ' > "$output_dir/count.txt"; rm -f /tmp/p6_tmp.txt
```

## P7 — Executables by month, disarmed (C1 style, probable)

**Problem.** Copy all executable files (recursive) into `output_dir/<Mon>/`
(`date +%b`), then `chmod -x` the copies.

**Solution.**
```bash
#!/bin/bash
input_dir="$1"; output_dir="$2"; mkdir -p "$output_dir"
while IFS= read -r f; do
  m=$(date -r "$f" +%b); mkdir -p "$output_dir/$m"
  cp -- "$f" "$output_dir/$m/" && chmod -x "$output_dir/$m/$(basename "$f")"
done < <(find "$input_dir" -type f -executable)
```

## P8 — Theory one-markers (quiz Part-B style, memorise)

- `exec < a; exec < b; read x` reads **b** (last redirect wins).
- `q=sh; echo 1.$q 2."$q" 3.'$q'` → `1.sh 2.sh 3.$q` (single quotes block expansion).
- `grep -c pat` counts **matching lines**, not total matches (`grep -o pat | wc -l` for total).
- `sort -t: -k1,1` = by name; `-k1,1nr` = numeric reverse (sizes); `ls -Sr` = size ascending.
- `find` needs `-type f` (skip dirs) and `2>/dev/null` (silence permission errors).
