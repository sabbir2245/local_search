#!/bin/bash
EXPECTED_FILE="input/output.txt"
TESTS="test1_i test2_i test3_i test4_i test5_i test6_i test7_i recursion1_i recursion2_i"
PASS=0
FAIL=0
RESULTS=""

for test in $TESTS; do
    COMPILER_OUTPUT=$(./icg_compiler input/${test}.c 2>&1)
    if [ $? -ne 0 ]; then
        RESULTS+="FAIL: $test (compiler crashed)\n"
        FAIL=$((FAIL+1))
        continue
    fi

    OPT_COUNT=$(echo "$COMPILER_OUTPUT" | grep -oP 'Peephole optimizations applied: \K[0-9]+')

    fasm code.asm /tmp/test_bin >/dev/null 2>&1
    if [ $? -ne 0 ]; then
        RESULTS+="FAIL: $test (assembly failed)\n"
        FAIL=$((FAIL+1))
        continue
    fi

    ACTUAL=$(chmod +x /tmp/test_bin && /tmp/test_bin 2>/dev/null)

    EXPECTED=$(awk -v t="$test" '
        $0 == t ".out" { found=1; next }
        found && /^-+$/ { next }
        found && /^[[:space:]]*$/ { found=0; next }
        found { print }
    ' "$EXPECTED_FILE")

    if [ "$ACTUAL" = "$EXPECTED" ]; then
        RESULTS+="PASS: $test (opts: ${OPT_COUNT:-0})\n"
        PASS=$((PASS+1))
    else
        RESULTS+="FAIL: $test (opts: ${OPT_COUNT:-0})\n"
        RESULTS+="  Expected:\n$(echo "$EXPECTED" | sed 's/^/    /')\n"
        RESULTS+="  Actual:\n$(echo "$ACTUAL" | sed 's/^/    /')\n"
        FAIL=$((FAIL+1))
    fi
done

echo -e "\n========== RESULTS =========="
echo -e "$RESULTS"
echo "============================="
echo "PASS: $PASS / $((PASS+FAIL))"
echo "FAIL: $FAIL / $((PASS+FAIL))"
echo -e "$RESULTS" > test_results.txt
echo "Results saved to test_results.txt"
