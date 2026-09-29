# 📝 The Problems

**Make your script executable using:**

```bash
chmod +x script_name.sh
```
**Run your script:**

```bash
./script_name.sh
```

## Problem 1: The Warmup Greeter
**Goal:** Write a script that asks for the user's name and prints a personalized greeting.

**Requirements:**

- Prompt the user with: "Enter your name: "
- Read the input into a variable.
- Print: "Hello, [Name]! Welcome to Bash scripting."

**Core Concepts:** `read` command, variables, basic string printing.

## Problem 2: Even or Odd?
**Goal:** Write a script that takes an integer as a command-line argument and determines if it is even or odd.

**Requirements:**

- Check if exactly one argument is passed. If not, print an error message and exit with status 1.
- Check if the number is divisible by 2.
- Print "[Number] is Even" or "[Number] is Odd" accordingly.

**Core Concepts:** Command-line arguments (`$1`, `$#`), if-else conditionals, arithmetic operations (`%` operator).

## Problem 3: Directory Clean-up & Backup
**Goal:** Write a script that checks if a directory named backup exists in the current folder. If it doesn't, create it. Then, move all files ending with .log into that directory.

**Requirements:**

- Check for the existence of the directory backup.
- Use conditional directory checking (`[ -d "backup" ]`).
- Create the directory if missing, and print "Creating backup directory..."
- Move .log files and print "Moving logs to backup..."
- If no .log files exist, handle it gracefully (it's okay if mv displays a warning, but extra points if you check first!).

**Core Concepts:** File test operators, `mkdir`, globbing, `mv`.

## Problem 4: Countdown Timer
**Goal:** Write a script that takes a positive integer as an argument and counts down to 0, printing each number with a 1-second delay, then prints "Blast off!".

**Requirements:**

- Use a loop (while or for).
- Use the sleep command to pause execution for 1 second between numbers.

**Output format:**

```text
5...
4...
3...
2...
1...
Blast off!
```
**Core Concepts:** Loops, arithmetic decrement, `sleep` command.

## Problem 5: Simple Calculator
**Goal:** Create a simple calculator that performs addition, subtraction, multiplication, or division based on user choice.

**Requirements:**

- Prompt the user to enter two numbers.
- Prompt the user to select an operation: +, -, *, /.
- Perform the calculation and output the result.
- (Bonus) Prevent division by zero and display an error.

**Core Concepts:** Arithmetic evaluation (`$(( ... ))` or `bc`), user interactive input, `case` statements.