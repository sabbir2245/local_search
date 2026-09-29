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
