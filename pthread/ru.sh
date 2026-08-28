#!/bin/bash

# Configuration
SOURCE_FILE="solve2_2.cpp"
EXEC_FILE="./c"
TEMP_INPUT="temp_input.txt"
OUTPUT_FILE="output.txt"

# Compile code
echo "Compiling $SOURCE_FILE..."
g++ -pthread -std=c++17 "$SOURCE_FILE" -o "$EXEC_FILE"

if [ $? -ne 0 ]; then
    echo "Compilation failed!"
    exit 1
fi

# Clear or create the output file
> "$OUTPUT_FILE"

# Define 5 test cases: (N M x y)
declare -a TEST_CASES=(
    "15 5 10 3"    # Baseline
    "20 1 5 2"     # Heavy station contention (M=1)
    "12 12 8 4"    # Single large unit (M=N)
    "16 4 1 20"    # Fast typing, slow logbook
    "12 3 25 1"    # Slow typing, fast logbook
)

echo "Running test cases..."
echo "========================================" >> "$OUTPUT_FILE"

case_num=1
for test in "${TEST_CASES[@]}"; do
    echo "Running Test Case $case_num: $test"
    
    echo "=== TEST CASE $case_num ($test) ===" >> "$OUTPUT_FILE"
    
    # Write input to temporary file
    echo "$test" > "$TEMP_INPUT"
    
    # Execute solver with input and temporary output redirection
    "$EXEC_FILE" "$TEMP_INPUT" "temp_run_out.txt"
    
    # Append test results to master output file
    cat "temp_run_out.txt" >> "$OUTPUT_FILE"
    echo -e "\n========================================\n" >> "$OUTPUT_FILE"
    
    ((case_num++))
done

# Cleanup temporary files
rm -f "$TEMP_INPUT" "temp_run_out.txt"

echo "Done! All results saved to $OUTPUT_FILE."