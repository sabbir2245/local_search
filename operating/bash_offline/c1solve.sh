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
