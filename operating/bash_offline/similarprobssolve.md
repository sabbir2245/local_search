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
