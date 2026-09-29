#!/usr/bin/bash

# ============================================
# BASH CHEATSHEET — 90% of what you'll use
# ============================================

# --- SHEBANG ---
# Always start with #!/usr/bin/bash (or /bin/bash)

# --- VARIABLES ---
name="Alice"
echo "$name"                  # Use $ to reference
readonly pi=3.14              # Read-only variable

# --- STRINGS ---
greeting="Hello $name"        # Interpolation
echo "${name}_suffix"         # Brace to delimit
echo ${#name}                 # Length: 5
echo "${name,,}"              # lowercase
echo "${name^^}"              # UPPERCASE
echo "${name/A/B}"            # Replace first A→B
echo "${name//l/X}"           # Replace all l→X
sub="${name:1:3}"             # Substring: "li"

# --- ARRAYS ---
arr=("a" "b" "c")
echo "${arr[0]}"              # First element
echo "${arr[@]}"              # All elements
echo "${#arr[@]}"             # Length
arr+=("d")                    # Append

# --- ARITHMETIC ---
x=5
y=3
sum=$(( x + y ))              # + - * / % **
(( x++ ))                     # Increment (no $ needed inside (()))

# --- READING INPUT ---
read -p "Enter name: " var    # Prompt + read
read -s -p "Password: " pw    # Silent (no echo)

# --- CONDITIONALS ---
if [ "$name" = "Alice" ]; then
  echo "Hi Alice"
elif [ "$name" = "Bob" ]; then
  echo "Hi Bob"
else
  echo "Who are you?"
fi

# Numeric comparisons (-eq -ne -lt -le -gt -ge)
if [ "$x" -gt 3 ]; then echo "x > 3"; fi

# Alternative: (( )) for arithmetic
if (( x > 3 )); then echo "x > 3"; fi

# File tests
if [ -f "file.txt" ]; then echo "file exists"; fi     # -d (dir), -e (exists), -s (non-empty), -r (readable), -w, -x, -z (empty string)

# --- LOOPS ---
# For over a list
for item in one two three; do
  echo "$item"
done

# C-style for
for (( i=0; i<5; i++ )); do
  echo "$i"
done

# While
count=3
while (( count > 0 )); do
  echo "$count"
  (( count-- ))
done

# Read file line by line
while IFS= read -r line; do
  echo "$line"
done < "file.txt"

# --- FUNCTIONS ---
greet() {
  local name="$1"             # $1, $2, ... for args
  echo "Hello, $name"
  return 0                    # 0 = success, non-zero = error
}
greet "World"

# --- COMMAND-LINE ARGS ---
# $0 = script name, $1 $2 ... = positional args
# $# = arg count, $@ = all args, $? = last exit code

if [ $# -lt 1 ]; then
  echo "Usage: $0 <arg1>"
  exit 1
fi

# --- EXIT CODES ---
# 0 = success, 1–255 = error
exit 0

# --- COMMAND SUBSTITUTION ---
now=$(date)                   # Capture stdout into variable
files_count=$(ls | wc -l)     # Preferred over backticks

# --- REDIRECTION ---
# command > file     (stdout to file, overwrite)
# command >> file    (stdout to file, append)
# command 2> file    (stderr to file)
# command &> file    (both stdout & stderr)
# command < file     (stdin from file)
# command1 | command2  (pipe)

# --- STRING TESTS ---
[ -z "$var" ]                 # True if empty
[ -n "$var" ]                 # True if not empty
[ "$a" = "$b" ]               # String equality
[ "$a" != "$b" ]              # String inequality

# --- COMBINING CONDITIONS ---
if [ "$a" = "$b" ] && [ "$c" = "$d" ]; then :; fi     # AND
if [ "$a" = "$b" ] || [ "$c" = "$d" ]; then :; fi     # OR

# --- CASE STATEMENT ---
case "$1" in
  start) echo "Starting..." ;;
  stop)  echo "Stopping..." ;;
  *)     echo "Usage: $0 {start|stop}" ; exit 1 ;;
esac

# --- ONE-LINERS ---
[ -d "backup" ] || mkdir backup   # Create dir if missing
cmd || echo "cmd failed"          # Run fallback on failure
cmd && echo "cmd succeeded"       # Run on success
