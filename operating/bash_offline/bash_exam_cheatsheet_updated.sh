BASH SCRIPTING EXAM CHEATSHEET — UPDATED FOR THIS YEAR'S QUESTIONS
======================================================================

This sheet is a standalone revision guide. It combines the core Bash topics
with the patterns that appeared in this year's exam.

======================================================================
0. STANDARD EXAM SCRIPT TEMPLATE
======================================================================

#!/usr/bin/bash

if [ "$#" -ne 2 ]; then
    echo "Usage: $0 <input_directory> <output_directory>"
    exit 1
fi

INPUT="${1%/}"
OUTPUT="${2%/}"

if [ ! -d "$INPUT" ]; then
    echo "Error: '$INPUT' is not a directory."
    exit 1
fi

mkdir -p "$OUTPUT"

# Main logic here

echo "Completed."

Important:
    Always quote paths:
        "$file" "$INPUT" "$OUTPUT" "$@"

    Remove a trailing slash when relative-path operations are needed:
        INPUT="${1%/}"

    Exit status:
        0       success
        nonzero failure


======================================================================
1. ARGUMENTS, VARIABLES, AND QUOTING
======================================================================

$0              Script name
$1, $2, ...     Positional arguments
$#              Number of arguments
"$@"            All arguments, each preserved separately
"$*"            All arguments joined into one string
$?              Exit status of previous command
$$              Current shell PID
$!              PID of latest background command

Assignment:
    name="Rafsan"          # Correct
    name = "Rafsan"       # Wrong

Command substitution:
    filename=$(basename "$file")
    line_count=$(wc -l < "$file")
    month=$(date -r "$file" +%b)

Default value:
    REPORT="${2:-report.txt}"

Remove trailing slash:
    INPUT="${INPUT%/}"

Why quote:
    cp "$file" "$destination"

Without quotes, spaces, wildcard characters, and empty variables may break the
command.


======================================================================
2. TESTS AND CONDITIONALS
======================================================================

FILE TESTS

[ -e "$path" ]     Exists
[ -f "$path" ]     Regular file
[ -d "$path" ]     Directory
[ -s "$path" ]     Nonempty file
[ -r "$path" ]     Readable
[ -w "$path" ]     Writable
[ -x "$path" ]     Executable

STRING TESTS

[ -z "$text" ]          Empty
[ -n "$text" ]          Nonempty
[ "$a" = "$b" ]         Equal
[ "$a" != "$b" ]        Not equal

INTEGER TESTS

[ "$a" -eq "$b" ]       Equal
[ "$a" -ne "$b" ]       Not equal
[ "$a" -lt "$b" ]       Less than
[ "$a" -le "$b" ]       Less than or equal
[ "$a" -gt "$b" ]       Greater than
[ "$a" -ge "$b" ]       Greater than or equal

Arithmetic form:
    if (( a > b )); then
        ...
    fi

Pattern matching:
    [[ "$filename" == FINAL_* ]]
    [[ "$filename" == *.txt ]]
    [[ "$answer" == [yY] ]]

Structure:
    if condition; then
        ...
    elif condition; then
        ...
    else
        ...
    fi

Logical operators:
    condition1 && condition2
    condition1 || condition2

Example:
    if (( winter > dragon && winter > throne )); then
        category="Stark"
    elif (( dragon > winter && dragon > throne )); then
        category="Targaryen"
    else
        category="Royal"
    fi

Tie handling:
    The final else can intentionally collect ties and unmatched cases.


swtich cases :

    

======================================================================
3. LOOPS
======================================================================

FOR LOOP OVER ARGUMENTS

    for file in "$@"; do
        echo "$file"
    done

FOR LOOP OVER FIXED VALUES

    for category in Stark Targaryen Royal; do
        echo "$category"
    done

FOR LOOP OVER GLOB

    for file in "$DIR"/*.txt; do
        [ -e "$file" ] || continue
        echo "$file"
    done

C-STYLE LOOP

    for ((i=0; i<${#files[@]}; i++)); do
        echo "${files[$i]}"
    done

READ A TEXT FILE

    while IFS= read -r line; do
        echo "$line"
    done < "$INPUT_FILE"

READ DELIMITED FIELDS

    while IFS='|' read -r id time message; do
        echo "$id $time $message"
    done < "$LOG_FILE"


======================================================================
4. SAFE RECURSIVE FILE TRAVERSAL
======================================================================

Best general form:

    while IFS= read -r -d '' file; do
        echo "$file"
    done < <(find "$INPUT" -type f -print0)

Meaning:
    find -print0      Separate paths with a null byte
    read -d ''        Read up to a null byte
    IFS=              Do not trim whitespace
    -r                Do not treat backslashes as escapes
    < <(...)          Process substitution

Avoid:
    for file in $(find "$INPUT" -type f)

It breaks paths containing spaces, tabs, or newlines.

A normal newline loop:

    while IFS= read -r file; do
        ...
    done < <(find "$INPUT" -type f)

This handles spaces but not filenames containing newline characters.

Piped while loop:

    find "$INPUT" -type f | while IFS= read -r file; do
        ((count++))
    done

The loop may run in a subshell, so count may be unchanged afterward.

Use process substitution when values must survive:

    while IFS= read -r file; do
        ((count++))
    done < <(find "$INPUT" -type f)


======================================================================
5. FIND COMMAND — IMPORTANT EXAM OPTIONS
======================================================================

Basic:
    find "$DIR" -type f
    find "$DIR" -type d
    find "$DIR" -maxdepth 1 -type f
    find "$DIR" -type f -name 'FINAL_*'
    find "$DIR" -type f -iname '*.txt'
    find "$DIR" -type f -executable
    find "$DIR" -type f -print0

Multiple extensions:
    find "$DIR" -type f \
        \( -iname '*.mp3' -o -iname '*.flac' \
           -o -iname '*.mp4' -o -iname '*.mkv' \) \
        -print0

Why parentheses:
    They group the OR conditions so -type f applies to every extension.

Wrong:
    find "$DIR" -type f -name '*.jpg' -o -name '*.png'

Better:
    find "$DIR" -type f \( -name '*.jpg' -o -name '*.png' \)

Exclude a generated directory:
    find "$INPUT" -type f ! -path "$OUTPUT/*"

GNU find formatted output:
    %p      Full path
    %f      Filename
    %s      Size in bytes
    %T@     Modification time as Unix-style fractional seconds
    %Tb     Modification month abbreviation, such as Jan
    \0      Null terminator

Examples:
    find "$DIR" -type f -printf '%s\t%p\0'
    find "$DIR" -type f -executable -printf '%Tb\t%p\0'


======================================================================
6. FILE METADATA
======================================================================

Filename:
    filename=$(basename "$file")

Parent directory:
    parent=$(dirname "$file")

Size:
    size=$(stat --format=%s "$file")

Modification time in seconds:
    modified=$(stat --format=%Y "$file")

Modification month:
    month=$(date -r "$file" +%b)

Current timestamp:
    now=$(date +%s)

Human-readable time:
    timestamp=$(date '+%Y-%m-%d %H:%M:%S')

Executable test:
    if [ -x "$file" ]; then
        ...
    fi

Find executable files:
    find "$INPUT" -type f -executable -print0


======================================================================
7. DETECT FILE TYPE BY CONTENT, NOT EXTENSION
======================================================================

The file command identifies content.

Basic:
    file "$file"

Description only:
    description=$(file -b "$file")

MIME type:
    mime=$(file -b --mime-type "$file")

Detect text files:
    if [[ "$mime" == text/* ]]; then
        echo "Text file"
    fi

Exam-style alternative:
    if file "$file" | grep -qi 'text'; then
        echo "Text file"
    fi

Difference:
    - Extension-based detection:
        [[ "$file" == *.txt ]]

    - Content-based detection:
        file "$file"

When the question says "regardless of extension", use file rather than the
filename extension.


======================================================================
8. GREP — MATCHING AND COUNTING
======================================================================

grep -i pattern file
    Case-insensitive matching.

grep -q pattern file
    Quiet; use only success/failure.

grep -c pattern file
    Count matching lines, not total occurrences.

grep -o pattern file
    Print each matching occurrence separately.

grep -w pattern file
    Match a whole word.

grep -F pattern file
    Treat pattern literally.

grep -x pattern file
    Match the entire line.

Useful combinations:
    grep -qi -- 'text' "$file"
    grep -qiw -- 'winter' "$file"
    grep -qxF -- "$value" "$list_file"

COUNT MATCHING LINES

    winter_lines=$(grep -ic -- 'winter' "$file")

If one line contains "winter winter", grep -c counts that line once.

COUNT TOTAL OCCURRENCES

    winter_count=$(grep -oiw -- 'winter' "$file" | wc -l)

Choose based on question wording:
    "number of lines containing keyword"  -> grep -c
    "number of keyword occurrences"       -> grep -o | wc -l


======================================================================
9. DOMINANT-KEYWORD CLASSIFICATION
======================================================================

Pattern:

    winter=$(grep -oiw -- 'winter' "$file" | wc -l)
    dragon=$(grep -oiw -- 'dragon' "$file" | wc -l)
    throne=$(grep -oiw -- 'throne' "$file" | wc -l)

    total=$((winter + dragon + throne))

    if (( winter > dragon && winter > throne )); then
        category="Stark"
    elif (( dragon > winter && dragon > throne )); then
        category="Targaryen"
    else
        category="Royal"
    fi

Important questions:
    1. Are comparisons strict?
    2. What happens on a tie?
    3. What happens when all counts are zero?
    4. Is matching case-sensitive?
    5. Count matching lines or occurrences?
    6. Match substrings or whole words?

Create category directories:
    mkdir -p "$OUTPUT"/{Stark,Targaryen,Royal}

Append score to filename:
    cp "$file" "$OUTPUT/$category/${filename}_${total}"

If the extension must remain last:

    stem="${filename%.*}"
    ext="${filename##*.}"

    if [ "$stem" = "$ext" ]; then
        new_name="${filename}_${total}"
    else
        new_name="${stem}_${total}.${ext}"
    fi


======================================================================
10. SORTING NUL-TERMINATED RECORDS
======================================================================

This is one of the most important new exam patterns.

Why NUL records:
    Paths may contain spaces.
    NUL separates complete records safely.

SORT PATHS ALPHABETICALLY

    find "$DIR" -type f -print0 | sort -z

SORT BY FILE SIZE

    find "$DIR" -type f -printf '%s\t%p\0' |
        sort -z -n -t $'\t' -k1,1

Read result:

    while IFS= read -r -d '' entry; do
        size="${entry%%$'\t'*}"
        file="${entry#*$'\t'}"
        echo "$size $file"
    done < <(
        find "$DIR" -type f -printf '%s\t%p\0' |
            sort -z -n -t $'\t' -k1,1
    )

SORT BY LINE COUNT USING AN ARRAY

    records=()

    while IFS= read -r -d '' file; do
        lines=$(wc -l < "$file")
        records+=("$lines"$'\t'"$file")
    done < <(find "$INPUT" -type f -print0)

    while IFS= read -r -d '' entry; do
        lines="${entry%%$'\t'*}"
        file="${entry#*$'\t'}"
        echo "$lines $file"
    done < <(
        printf '%s\0' "${records[@]}" |
            sort -z -n -t $'\t' -k1,1
    )

Why tab instead of underscore:
    A path may contain underscores.
    A tab delimiter is less likely to conflict and is easy to remove.

Simple exam version:
    records+=("${lines}_${file}")
    file="${entry#*_}"

It usually works because only the first underscore is removed, but tab is
clearer and safer.

Important sort options:
    -z      NUL-terminated records
    -n      Numeric sort
    -r      Reverse
    -t      Field delimiter
    -k1,1   Sort only by field 1


======================================================================
11. NUMBER FILES IN SORTED ORDER
======================================================================

Generic pattern:

    count=0

    while IFS= read -r -d '' entry; do
        file="${entry#*$'\t'}"
        filename=$(basename "$file")

        cp "$file" "$OUTPUT/${count}_$filename"
        ((count++))
    done < <(
        find "$INPUT" -type f -printf '%s\t%p\0' |
            sort -z -n -t $'\t' -k1,1
    )

Start at 1:
    count=1

Zero-padded prefix:
    prefix=$(printf '%03d' "$count")
    cp "$file" "$OUTPUT/${prefix}_$filename"

Important:
    Ascending numerical prefixes can mean 0,1,2 or 1,2,3.
    Follow the sample output or specification.


======================================================================
12. ARRAYS
======================================================================

Indexed array:
    files=()
    files+=("$file")

All elements:
    printf '%s\n' "${files[@]}"

Length:
    count="${#files[@]}"

Loop:
    for file in "${files[@]}"; do
        echo "$file"
    done

Read file lines:
    mapfile -t lines < "$INPUT_FILE"

Read newline-separated text:
    mapfile -t values <<< "$text"

NUL-output from array:
    printf '%s\0' "${files[@]}"

Critical quoting rule:
    Correct:
        manifest+=("$relative")

    Wrong:
        manifest+=($relative)

The unquoted form performs word splitting and wildcard expansion.


======================================================================
13. ASSOCIATIVE ARRAYS
======================================================================

Declare:
    declare -A catalog
    declare -A counts

Store:
    counts["$category"]=1

Increment:
    ((counts["$category"]++))

Append newline-separated values:
    catalog["$artist"]+="$title"$'\n'

All keys:
    printf '%s\n' "${!catalog[@]}"

All values:
    printf '%s\n' "${catalog[@]}"

Sort keys:
    while IFS= read -r key; do
        echo "$key"
    done < <(printf '%s\n' "${!catalog[@]}" | sort)

Print grouped sorted report:

    : > "$OUTPUT_FILE"

    while IFS= read -r artist; do
        echo "$artist" >> "$OUTPUT_FILE"

        printf '%s' "${catalog[$artist]}" |
            sort >> "$OUTPUT_FILE"
    done < <(printf '%s\n' "${!catalog[@]}" | sort)


======================================================================
14. PARAMETER EXPANSION FOR PATHS AND FILENAMES
======================================================================

Remove shortest prefix:
    "${value#pattern}"

Remove longest prefix:
    "${value##pattern}"

Remove shortest suffix:
    "${value%pattern}"

Remove longest suffix:
    "${value%%pattern}"

Filename:
    filename="${file##*/}"

Remove extension:
    stem="${filename%.*}"

Get extension:
    extension="${filename##*.}"

Text after final underscore:
    value="${filename##*_}"

Text before first underscore:
    value="${filename%%_*}"

Relative path:
    relative="${file#"$INPUT"/}"

This is better than:
    directory="${directory#*$INPUT/}"

because the quoted prefix states exactly what must be removed.

Preserve relative structure:

    relative="${file#"$INPUT"/}"
    destination="$OUTPUT/package/$relative"

    mkdir -p "$(dirname "$destination")"
    cp "$file" "$destination"


======================================================================
15. PRESERVE DIRECTORY STRUCTURE
======================================================================

Scenario:
    Copy every FINAL_* file into OUTPUT/package while preserving its path
    relative to INPUT.

Template:

    INPUT="${1%/}"
    OUTPUT="${2%/}"

    mkdir -p "$OUTPUT/package"

    while IFS= read -r -d '' file; do
        relative="${file#"$INPUT"/}"
        destination="$OUTPUT/package/$relative"

        mkdir -p "$(dirname "$destination")"
        cp "$file" "$destination"
    done < <(find "$INPUT" -type f -name 'FINAL_*' -print0)

Example:
    Input:
        input/teamA/docs/FINAL_report.pdf

    Relative:
        teamA/docs/FINAL_report.pdf

    Destination:
        output/package/teamA/docs/FINAL_report.pdf


======================================================================
16. MANIFEST AND COUNT FILES
======================================================================

Collect relative paths:

    manifest=()
    count=0

    while IFS= read -r -d '' file; do
        relative="${file#"$INPUT"/}"

        manifest+=("$relative")
        ((count++))
    done < <(find "$INPUT" -type f -name 'FINAL_*' -print0)

Write count:
    printf '%s\n' "$count" > "$OUTPUT/count.txt"

Write sorted manifest:

    printf '%s\0' "${manifest[@]}" |
        sort -z |
        tr '\0' '\n' > "$OUTPUT/manifest.txt"

Alternative without tr:

    : > "$OUTPUT/manifest.txt"

    while IFS= read -r -d '' entry; do
        printf '%s\n' "$entry" >> "$OUTPUT/manifest.txt"
    done < <(printf '%s\0' "${manifest[@]}" | sort -z)

Clear/create:
    : > "$file"

Equivalent:
    > "$file"


======================================================================
17. CONTENT-BASED TEXT FILE COLLECTION
======================================================================

Scenario:
    Find every text file regardless of extension, sort by line count, and copy
    with numerical prefixes.

Template:

    records=()

    while IFS= read -r -d '' file; do
        mime=$(file -b --mime-type "$file")

        if [[ "$mime" == text/* ]]; then
            lines=$(wc -l < "$file")
            records+=("$lines"$'\t'"$file")
        fi
    done < <(find "$INPUT" -type f -print0)

    count=0

    while IFS= read -r -d '' entry; do
        file="${entry#*$'\t'}"
        filename=$(basename "$file")

        cp "$file" "$OUTPUT/${count}_$filename"
        ((count++))
    done < <(
        printf '%s\0' "${records[@]}" |
            sort -z -n -t $'\t' -k1,1
    )


======================================================================
18. EXECUTABLE FILES GROUPED BY MODIFICATION MONTH
======================================================================

Template:

    while IFS= read -r -d '' file; do
        filename=$(basename "$file")
        month=$(date -r "$file" +%b)

        mkdir -p "$OUTPUT/$month"
        cp "$file" "$OUTPUT/$month/$filename"
        chmod a-x "$OUTPUT/$month/$filename"
    done < <(find "$INPUT" -type f -executable -print0)

Using find -printf:

    while IFS= read -r -d '' entry; do
        month="${entry%%$'\t'*}"
        file="${entry#*$'\t'}"
        filename=$(basename "$file")

        mkdir -p "$OUTPUT/$month"
        cp "$file" "$OUTPUT/$month/$filename"
        chmod a-x "$OUTPUT/$month/$filename"
    done < <(
        find "$INPUT" -type f -executable -printf '%Tb\t%p\0'
    )

chmod:
    chmod a-x "$file"      Remove execute permission for everyone
    chmod +x "$file"       Add execute permission
    chmod u+x "$file"      Add execute permission for owner


======================================================================
19. REGEX AND FILENAME PARSING
======================================================================

Bash regex:
    if [[ "$text" =~ REGEX ]]; then
        echo "${BASH_REMATCH[0]}"
        echo "${BASH_REMATCH[1]}"
    fi

Pattern: Artist - Title

    ^(.+)[[:space:]]+-[[:space:]]+(.+)$

Pattern: Title (2025) - Artist

    ^(.+)[[:space:]]+\([0-9]{4}\)[[:space:]]+-[[:space:]]+(.+)$

Use:

    name="${filename%.*}"
    artist="Unknown"
    title="Unknown"

    if [[ "$name" =~ ^(.+)[[:space:]]+\([0-9]{4}\)[[:space:]]+-[[:space:]]+(.+)$ ]]; then
        title="${BASH_REMATCH[1]}"
        artist="${BASH_REMATCH[2]}"
    elif [[ "$name" =~ ^(.+)[[:space:]]+-[[:space:]]+(.+)$ ]]; then
        artist="${BASH_REMATCH[1]}"
        title="${BASH_REMATCH[2]}"
    fi

Important:
    Put the more specific pattern first.
    Otherwise the general Artist - Title pattern may capture the wrong form.

Do not quote a regex variable on the right side:
    regex='^[0-9]+$'
    [[ "$value" =~ $regex ]]


======================================================================
20. REPORT GENERATION
======================================================================

Overwrite a report:
    {
        echo "Total: $total"
        echo "Processed: $processed"
    } > "$REPORT"

Append:
    echo "$line" >> "$REPORT"

Clear:
    : > "$REPORT"

Sorted report:
    {
        for key in "${!counts[@]}"; do
            printf '%s: %s\n' "$key" "${counts[$key]}"
        done
    } | sort > "$REPORT"

Manifest:
    printf '%s\n' "$relative" >> "$MANIFEST"


======================================================================
21. TEXT-PROCESSING COMMANDS
======================================================================

grep
    grep -i       case-insensitive
    grep -q       quiet
    grep -c       count matching lines
    grep -o       output each match
    grep -w       whole word
    grep -x       whole line
    grep -F       literal text

sort
    sort          alphabetic
    sort -n       numeric
    sort -r       reverse
    sort -u       unique
    sort -z       NUL records
    sort -k2,2    sort by field 2
    sort -t ':'   use : as delimiter

uniq
    sort file | uniq
    sort file | uniq -c

wc
    wc -l < "$file"
    wc -w < "$file"
    wc -c < "$file"

cut
    cut -d ':' -f 1
    md5sum "$file" | cut -d ' ' -f 1

sed
    sed 's/old/new/' file
    sed 's/old/new/g' file
    sed -i -E 's/(old)/new/g' file

head / tail / tac
    head -n 5 file
    tail -n 5 file
    tac file

tr
    tr 'a-z' 'A-Z'
    tr -d ' '
    tr '\0' '\n'

printf
    printf '%s\n' "$value"
    printf '%04d\n' 7


======================================================================
22. FILE AND DIRECTORY COMMANDS
======================================================================

basename "$path"             Get filename
dirname "$path"              Get parent path
mkdir -p "$dir"              Create path if needed
cp "$src" "$dst"             Copy file
cp -r "$src" "$dst"          Copy directory recursively
mv "$src" "$dst"             Move or rename
rm "$file"                   Delete file
rm -r "$dir"                 Delete directory recursively
touch "$file"                Create/update file
chmod a-x "$file"            Remove executable permissions
stat --format=%s "$file"     Size
stat --format=%Y "$file"     Modification timestamp
date -r "$file" +%b          Modification month
file "$file"                 Detect content type


======================================================================
23. IMPORTANT CORRECTIONS TO THIS YEAR'S SUBMITTED SOLUTIONS
======================================================================

1. GREP COUNTING

    grep -ic winter "$file"

counts matching lines, not total occurrences.

For occurrences:
    grep -oiw -- 'winter' "$file" | wc -l


2. NUMERIC SORT

    sort -z

is alphabetic unless records happen to sort correctly.

For size or line count:
    sort -z -n


3. ARRAY APPEND MUST BE QUOTED

Wrong:
    manifest+=($directory/$filename)

Correct:
    manifest+=("$directory/$filename")

Otherwise spaces split one path into multiple array elements.


4. RELATIVE PATH EXTRACTION

Fragile:
    directory="${directory#*$INPUT/}"

Better:
    relative="${file#"$INPUT"/}"


5. DELIMITER CHOICE

Possible:
    records+=("${lines}_${file}")

Safer:
    records+=("$lines"$'\t'"$file")

A filename may contain underscores.


6. SAFE FIND LOOP

Usable for normal names:
    find ... | while IFS= read -r file

Safest:
    while IFS= read -r -d '' file; do
        ...
    done < <(find ... -print0)


7. OUTPUT COLLISIONS

Two source files in different folders may have the same basename.

This:
    cp "$file" "$OUTPUT/$filename"

may overwrite one file.

Solutions depend on specification:
    - Preserve relative paths.
    - Add a prefix.
    - Add a counter.
    - Detect an existing destination.


8. EXTENSION POSITION

This:
    file.txt_5

puts the score after the extension.

Sometimes required:
    file_5.txt

Read the required naming format carefully.


9. COUNT INITIAL VALUE

    count=0

produces 0_, 1_, 2_.

Use count=1 if examples begin at 1.


10. SET -e AND POST-INCREMENT

With set -e, this can stop the script when count is initially zero:
    ((count++))

Safer under set -e:
    ((++count))

Most exam scripts do not use set -e, but know the difference.


======================================================================
24. COMMON MISTAKES
======================================================================

1. Unquoted variables:
    cp $file $output

2. Spaces around assignment:
    name = "abc"

3. Unsafe find loop:
    for file in $(find ...)

4. Losing variables in piped while loop.

5. Forgetting -print0 with read -d ''.

6. Forgetting -n for numeric sorting.

7. Forgetting to quote array appends.

8. Using grep -c when occurrences are required.

9. Using extension when content detection is required.

10. Forgetting parentheses around find OR conditions.

11. Using basename when relative structure must be preserved.

12. Overwriting files with duplicate basenames.

13. Removing the wrong prefix with parameter expansion.

14. Treating integers as strings:
    [ "$a" > "$b" ]        # Wrong for numeric comparison

15. Leading-zero arithmetic:
    number=$((10#$number))

16. Forgetting to clear a report before appending:
    : > "$REPORT"

17. Quoting the right side of =~ incorrectly.

18. Forgetting declare -A for associative arrays.

19. Using "$*" where "$@" is needed.

20. Using return to terminate the whole script; use exit.


======================================================================
25. LAST-MINUTE MEMORIZATION LIST
======================================================================

1. Validate arguments:
    [ "$#" -ne 2 ] && exit 1

2. Validate directory:
    [ -d "$INPUT" ] || exit 1

3. Safe recursive traversal:
    while IFS= read -r -d '' file; do ...; done \
        < <(find "$INPUT" -type f -print0)

4. Detect text by content:
    [[ "$(file -b --mime-type "$file")" == text/* ]]

5. Count lines:
    lines=$(wc -l < "$file")

6. Count matching lines:
    grep -ic -- 'word' "$file"

7. Count occurrences:
    grep -oiw -- 'word' "$file" | wc -l

8. File size:
    size=$(stat --format=%s "$file")

9. Modification month:
    month=$(date -r "$file" +%b)

10. Executable files:
    find "$INPUT" -type f -executable -print0

11. Sort numeric NUL records:
    sort -z -n

12. Size-path record:
    printf '%s\t%s\0' "$size" "$file"

13. Parse first tab:
    value="${entry%%$'\t'*}"
    file="${entry#*$'\t'}"

14. Relative path:
    relative="${file#"$INPUT"/}"

15. Preserve tree:
    mkdir -p "$(dirname "$OUTPUT/$relative")"

16. Manifest append:
    manifest+=("$relative")

17. Associative array:
    declare -A catalog

18. Group values:
    catalog["$key"]+="$value"$'\n'

19. Sort associative keys:
    printf '%s\n' "${!catalog[@]}" | sort

20. Regex capture:
    [[ "$name" =~ REGEX ]]
    "${BASH_REMATCH[1]}"

21. Remove extension:
    stem="${filename%.*}"

22. Copy with prefix:
    cp "$file" "$OUTPUT/${count}_$filename"

23. Zero padding:
    prefix=$(printf '%03d' "$count")

24. Clear output file:
    : > "$OUTPUT_FILE"

25. Always quote:
    "$file" "$INPUT" "$OUTPUT" "${array[@]}"


======================================================================
26. FUNCTIONS
======================================================================

Define:
    usage() {
        echo "Usage: $0 <input> <output>"
        exit 1
    }

    function usage {
        echo "Usage: $0 <input> <output>"
        exit 1
    }

Call:
    usage

Local variables (scope limited to the function):
    myfunc() {
        local var="visible only inside"
        echo "$var"
    }

Return value:
    myfunc() {
        return 42
    }
    myfunc
    echo $?        # 42

Return early (not exit the whole script):
    myfunc() {
        [ -z "$1" ] && return 1
        echo "$1"
    }


======================================================================
27. CASE STATEMENTS
======================================================================

Basic:
    case "$var" in
        start)
            echo "Starting..."
            ;;
        stop)
            echo "Stopping..."
            ;;
        *)
            echo "Unknown option"
            ;;
    esac

Match multiple patterns:
    case "$ans" in
        y|Y|yes|Yes)
            echo "Confirmed"
            ;;
        n|N|no|No)
            echo "Cancelled"
            ;;
        *)
            echo "Invalid input"
            ;;
    esac

Default value with case:
    case "${1:-help}" in
        init)  init_repo ;;
        add)   add_files ;;
        help)  usage ;;
        *)     echo "Unknown command" ;;
    esac

Fallthrough (intentional, no break needed — just omit ;;):
    case "$x" in
        a)
            echo "a"
            ;&        # continues into next pattern
        b)
            echo "a or b"
            ;;
    esac


======================================================================
28. HERE DOCUMENTS AND HERE STRINGS
======================================================================

Here document (multi-line block):
    cat <<EOF
    This is a multi-line
    block of text.
    EOF

Quoted delimiter prevents variable expansion:
    cat <<'EOF'
    $HOME is literal, not expanded
    EOF

Indented here document (using <<- with tabs):
    cat <<-EOF
        This line must be indented with a TAB (not spaces).
    EOF

Capture into variable:
    help_text=$(cat <<'EOF'
    Usage: script <input> <output>
    EOF
    )

Append to file:
    cat >> "$FILE" <<EOF
    new line of text
    EOF

Here string (pass string as stdin):
    grep "foo" <<< "$variable"
    read -r first <<< "$line"


======================================================================
29. READ COMMAND OPTIONS
======================================================================

Basic:
    read name

With prompt:
    read -p "Enter name: " name

With prompt and silent input (password):
    read -s -p "Enter password: " pass

Read into array:
    read -a arr -p "Enter numbers: "
    echo "${arr[0]}"

Read with timeout (seconds):
    read -t 5 -p "Quick! Answer: " ans

Read exactly N characters:
    read -n 1 -p "Press y or n: " ans


======================================================================
30. TAC, REV, DIFF
======================================================================

tac — reverse cat (last line first):
    tac "$LOG"                              # latest commit first

rev — reverse each line character-wise:
    echo "hello" | rev                      # olleh

diff — compare files:
    diff -q file1 file2                     # quiet, just exit status
    diff -u file1 file2                     # unified diff format
    diff -u --label "old" --label "new" f1 f2   # custom labels
    diff -q "$snap" "$file" >/dev/null 2>&1 # check if different (exit 0 = same)


======================================================================
31. STDERR REDIRECTION
======================================================================

Send errors to /dev/null:
    find "$DIR" -type f 2>/dev/null

Redirect stderr to stdout:
    command 2>&1
    command &> file                         # both stdout and stderr

Append both to file:
    command &>> file

Write stderr message:
    echo "Error: not found" >&2

Suppress both streams:
    command >/dev/null 2>&1

Order matters:
    command 2>&1 > file   # WRONG — stderr still goes to terminal
    command > file 2>&1   # CORRECT — both go to file


======================================================================
32. ADDITIONAL FILE TESTS
======================================================================

[ -s "$file" ]    File exists and is non-empty
[ -L "$file" ]    File is a symbolic link
[ -S "$file" ]    File is a socket
[ -p "$file" ]    File is a named pipe (FIFO)
[ file1 -nt file2 ]  file1 is newer than file2
[ file1 -ot file2 ]  file1 is older than file2


======================================================================
33. GREP EXTRA PATTERNS
======================================================================

Exact line + literal string (no regex):
    grep -xF "$exact_line" "$file"

Count matching files (not lines):
    grep -l -- 'pattern' "$DIR"/* | wc -l

Invert match:
    grep -v 'pattern' "$file"

Show only filename:
    grep -l 'pattern' "$file"

Show line number:
    grep -n 'pattern' "$file"


======================================================================
34. CONTINUE, BREAK, SHIFT
======================================================================

continue — skip to next iteration:
    for file in "$@"; do
        [ ! -f "$file" ] && continue
        echo "$file"
    done

break — exit loop early:
    while IFS= read -r line; do
        [ -z "$line" ] && break
        echo "$line"
    done < "$file"

shift — discard first argument(s):
    shift           # $2 becomes $1, $3 becomes $2, etc.
    shift 2         # discard first 2 arguments

Used for subcommand dispatch:
    case "$1" in
        add)
            shift
            add_files "$@"          # pass remaining args
            ;;
    esac


======================================================================
35. RETURN VS EXIT
======================================================================

return   End a function; set $? but keep script running
exit     End the entire script

    check_file() {
        [ -f "$1" ] && return 0     # OK, continue script
        return 1                    # failed, continue script
    }

    die() {
        echo "Fatal: $1" >&2
        exit 1                      # stop everything
    }

return only valid inside a function or sourced script.
exit is valid anywhere.


======================================================================
36. ZERO-PADDING WITH PRINTF
======================================================================

    printf '%04d' 7          # 0007
    printf '%02d' 1          # 01
    printf '%d' 42           # 42 (no padding)

Store in variable:
    padded=$(printf '%03d' "$count")

Common exam use — sequential file naming:
    new_id=$(printf '%04d' $((log_lines + 1)))
    prefix=$(printf '%03d' "$count")
    cp "$file" "$OUTPUT/${prefix}_$filename"


======================================================================
37. SET OPTIONS (SHELL BEHAVIOR)
======================================================================

set -e     Exit on first error (errexit)
set -u     Treat unset variables as error (nounset)
set -x     Print commands before executing (xtrace, for debugging)
set -o pipefail   Pipe fails if any command in the pipe fails

Combine:
    set -euo pipefail

Disable:
    set +e   # allow errors again

Note: Under set -e, ((count++)) exits when count is 0 (since result is 0 / false).
Use ((++count)) or count=$((count + 1)) instead.

set -x example output:
    + echo 'hello world'
    hello world


======================================================================
38. TRAP (SIGNAL HANDLING)
======================================================================

Cleanup on exit:
    cleanup() {
        rm -f /tmp/tempfile.$$
    }
    trap cleanup EXIT

Ignore SIGINT (Ctrl+C):
    trap '' INT

Reset signal handler:
    trap - EXIT

Multiple signals:
    trap 'rm -f "$tmpf"' EXIT INT TERM

Print line number on error (debugging):
    trap 'echo "Error at line $LINENO"' ERR


======================================================================
39. /DEV/NULL USAGE
======================================================================

Discard output:
    command > /dev/null

Discard errors:
    command 2> /dev/null

Discard both:
    command > /dev/null 2>&1

Test if command succeeds (no output):
    if diff -q a b > /dev/null 2>&1; then
        echo "Files are identical"
    fi

Empty input (diff against nothing):
    diff -u "$snapshot" /dev/null


======================================================================
40. SED ALTERNATIVE DELIMITERS
======================================================================

When the pattern contains /, use a different delimiter:

    sed 's|old/path|new/path|' file      # pipe
    sed 's_old/path_new/path_' file      # underscore
    sed 's,old/path,new/path,' file      # comma
    sed 's#old/path#new/path#' file      # hash

Common exam use — strip ./ prefix:
    sed 's|^\./||'


======================================================================
41. DIFFERENCE BETWEEN ( ) AND { }
======================================================================

( ) — subshell: changes inside don't affect parent
    (cd /tmp && ls)     # current dir unchanged after

{ } — current shell: changes persist
    { cd /tmp && ls; }  # current dir is now /tmp

Semicolon before } is required.

Useful for grouping redirects:
    {
        echo "line 1"
        echo "line 2"
    } > "$OUTPUT_FILE"


======================================================================
42. EXTRA COMMANDS SEEN IN EXAM SCRIPTS
======================================================================

: > file         Create or truncate file (alias for true; succeeds always)
cat > file       Write interactively (Ctrl+D to end)
readlink -f      Resolve full path (canonicalize)
realpath         Resolve full path
mktemp           Create a safe temporary file /tmp/tmp.XXXXXX
dirname          Get parent directory path
md5sum / sha1sum / sha256sum    Compute file hash
tee              Write to file and stdout simultaneously
paste            Merge lines from multiple files
comm             Compare two sorted files line by line
join             Join lines of two files on a common field
fold             Wrap long lines
nl               Number lines (like cat -n)
tsort            Topological sort
expand / unexpand  Convert tabs to spaces / spaces to tabs
