#!/bin/bash
g++ -std=c++11 -Wall -Wextra 2205040_symbol_table.cpp -o symbol_table
if [ $? -eq 0 ]; then
    echo "Starting.."
    ./symbol_table sample_input.txt new_output.txt
fi
