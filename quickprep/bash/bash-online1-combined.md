# Bash — Online 1 Combined (from Downloads `Online 1(Bash)` zip)
Sources (in order): `a1files.zip` (A1 log analysis), `a2files.zip` (A2 email monitor), `onlineB1.zip` (B1 command runner), `b2files.zip` (B2 academic dirs)


---

<!-- ===== A1 — extract_access.sh ===== -->

# A1 — Log Access Counter (`extract_access.sh`)

## Problem statement (from `a1files.zip` `onlineA1.pdf`)

```text
README.md

2024-09-16

Problem Description:
You need to analyze log files that record system access history. Each log contains a timestamp, username,
and access type (e.g., "Login", "Logout", "File Access"). Your task is to count how many times each user
accessed the system within a specific time range. Do not miss reading the instructions below.

Requirements:
1. Write a script called extract_access.sh that:
Accepts a log file, an access type (e.g., "Login"), and a time range (startHH-endHH). Both ranges
are inclusive and are in 24-hour format.
Example usage: ./extract_access.sh access.log "Login" "01-05"
Filters lines where the access type matches and the time falls within the range. Example output:

johndoe Login
sidney Login
johndoe Login
...

2. Now pipe the output of extract_access.sh such that:
Counts how many times each user accessed the system.
Each line first contains the count of the user's access, followed by the username.

5 johndoe
3 sidney
...

This means that johndoe accessed the system 5 times and sidney accessed the system 3 times.

Instructions:
Use cut to extract columns or fields. For example, cut -d' ' -f1 will extract the first field from
each line, assuming that fields are separated by spaces. You can also extract multiple fields by
specifying a range, like this: cut -d' ' -f1-3.
Numbers with leading zeros are treated as octal by bash. To avoid this, prefix the variables with a 10#
to ensure that they are treated as decimal numbers. For example, if $x contains 08, you can convert it
to a decimal number like this: x=$((10#$x)).
The uniq -c command only counts adjacent duplicates, so be sure to sort the output before using
uniq -c to get the correct count of each user's access.
```

Page 2:

```text
Once you have found the frequency of each user's access, you may notice some leading whitespace in
the output. You can remove this whitespace using the sed command. It works like this: sed
's/<pattern>/<replacement>/'. For example, sed 's/^ *//' will remove leading whitespace
from each line.
```

Log format (`access.log`):

```text
2023-09-01 10:24:21 sidney Logout
2023-09-01 10:47:53 sidney Login
2023-09-01 09:28:31 johndoe Login
```

So per line: `date(1) time(2) username(3) action(4-)`. The hour is field 1 of the time field.

### A1 solution (`extract_access.sh`, from `a1files.zip`)

```bash
#!/usr/bin/bash


## First give execute permission to the script: chmod +x extract_access.sh
## RUN: ./extract_access.sh access.log "Login" "00-23" | sort | uniq -c | sort -nr | sed 's/^ *//' | cut -d' ' -f1,2
## The part after sort -nr just cleans up the output to match the expected output

if [ $# -ne 3 ]; then
    echo "Usage: ./extract_access.sh <log_file> <access_type> <timerange>"
    exit 1
fi

log_file=$1
access_type=$2
start_hh=$(echo "$3" | cut -d'-' -f1)
end_hh=$(echo "$3" | cut -d'-' -f2)

# Convert hours to integers
start_hh=$((10#$start_hh))
end_hh=$((10#$end_hh))

# Read and filter log file
while read -r line; do
    timestamp=$(echo "$line" | cut -d' ' -f2)
    username=$(echo "$line" | cut -d' ' -f3)
    action=$(echo "$line" | cut -d' ' -f4-) # Notice the - after 4
    hh=$(echo "$timestamp" | cut -d':' -f1)
    hh=$((10#$hh))

    # Check if action matches and time is within range
    if [[ "$action" == "$access_type" ]] && (( hh >= start_hh && hh <= end_hh )); then
        echo "$username $action"
    fi
done < "$log_file"
```

Key points:

- `cut -d' ' -f4-` grabs the whole action (`File Access` is two words).
- `$((10#$hh))` forces decimal so `08`/`09` don't break as octal.
- The counting pipeline is: `sort | uniq -c | sort -nr | sed 's/^ *//' | cut -d' ' -f1,2` — sort first (uniq only counts adjacent dupes), numeric-reverse sort by count, strip `uniq -c` padding, keep `count username`.


---

<!-- ===== A2 — email monitor ===== -->

# A2 — Suspicious Email Monitor (`receive_email.sh`)

## Problem statement (from `a2files.zip` `onlineA2.pdf`)

```text
README.md

2024-09-18

Problem Description:
You are tasked with monitoring email communication to detect suspicious content. The first script will
simulate sending an email, and the second script will simulate receiving and processing the email. The
emails are stored in a file called email.log, which you will keep monitoring for new emails. If the email is
from a suspicious source (e.g., an "enemy"), the second script should trigger an alert showing the sender's
name. The first script is provided for you, and you need to write the second script. You will trigger the
alert if the sender is from enemy@**.**.
Write a script that:
Monitors the email.log file for new emails indefinitely (see the hint below).
Checks if the sender is from enemy@**.**. If this condition is met, the script should trigger an alert
and display the sender's name and the email content.
The script should keep running until the user stops it with Ctrl+C.
Do not modify the first script. If the email.log file does not exist, your script should create it.
Example usage for the first script:
./send_email.sh <receiver> <sender> <message>
For example:
./send_email.sh agent@example.com "enemy@example.com" "This is a secret mission"
The script you write will keep running till you press Ctrl+C, and the first script will be used to send
emails.

Sample Output
If the email is from an enemy, the script should display an alert with the email content like this:

ALERT: Email received from enemy@example.com
Timestamp: 2024-09-17 20:03:38
From: enemy@example.com
To: alice@example.com
Body: Let's kidnap Mosfet and hold him for ransom.
---

Hint: tail -F email.log will keep the script running and monitor the file for new emails. To monitor the
file from your bash script, you can pipe the output of this command to a while loop and read line by line.
The end of each email is marked by a line containing ---.
```

Provided sender (`send_email.sh`, do not modify):

```bash
#!/bin/bash

# Usage: ./send_email.sh <recipient> <sender> <body>
# Example: ./send_email.sh agent@example.com "enemy@dummy.com" "This is a secret mission"

if [ "$#" -ne 3 ]; then
  echo "Usage: $0 <recipient> <sender> <body>"
  exit 1
fi

RECIPIENT=$1
SENDER=$2
BODY=$3

# Get the current timestamp
TIMESTAMP=$(date +"%Y-%m-%d %H:%M:%S")

# Log the email details to email.log
{
  echo "Timestamp: $TIMESTAMP"
  echo "From: $SENDER"
  echo "To: $RECIPIENT"
  echo "Body: $BODY"
  echo "---"
} >> email.log

echo "Email sent and logged."
```

### A2 solution (`receiver.sh`, from `a2files.zip`)

```bash
#!/usr/bin/bash

# Usage: ./receive_email.sh
# Continuously reads and monitors the log file for alerts

LOG_FILE="email.log"
SENDER_ALERT="enemy"


# Ensure the log file exists
if [ ! -f "$LOG_FILE" ]; then
    touch "$LOG_FILE"
    echo "Log file created: $LOG_FILE"
fi
declare -a content
flag=0
# Tail the log file and keep monitoring in real-time
tail -F "$LOG_FILE" | while read -r line; do

    content+=("$line")

    # If the line is ---, then the email content has ended
    if [[ "$line" == "---" ]]; then
        if (( flag == 1 )); then
            for i in "${content[@]}"; do
                echo "$i"
            done
            flag=0
        fi
        content=()
    fi

    if [[ "$line" == From:* ]]; then
        SENDER=$(echo "$line" | cut -d' ' -f2 | cut -d'@' -f1)

        if [[ "$SENDER" == "$SENDER_ALERT" ]]; then
            echo "ALERT: Email received from $SENDER!"
            flag=1
        fi
    fi

done
```

Key points:

- `tail -F` piped into `while read` keeps monitoring indefinitely until Ctrl+C.
- Each email block is buffered in the `content` array; it is printed only if the `From:` line matched `enemy@...` (flag set).
- `---` marks end-of-email: flush-or-discard, then reset the buffer.


---

<!-- ===== B1 — command_runner.sh ===== -->

# B1 — Command Runner with LED Alert (`command_runner.sh`)

## Problem statement (from `onlineB1.zip` `onlineB1.pdf`)

```text
README.md

2024-09-15

Problem Description
You are tasked with writing a script that continually asks for command input from the user. The script
should execute the command and display the output. If the command is exit, the script should terminate.
If a command fails to execute (like cd-ing into a non-existent directory), the script should display an error
message and blink the caps lock key LED on the keyboard.
You can toggle the values in /sys/class/leds/input24::capslock/brightness to turn the caps
lock LED on and off. A value of 1 turns the LED on, and a value of 0 turns the LED off. The path may vary
depending on the system, so you may need to adjust it accordingly. Specifically, only the numeric part in the
folder name input24::capslock may change, while the rest of the path should remain the same.

Sample Usage
You should run your script with sudo privileges to access the LED device. Without sudo, the script will not
be able to turn the LED on and off.

sudo ./command_runner.sh
Enter a command: ls nai
ls: cannot access 'nai': No such file or directory
Failed: Command exited with status 2.
Enter a command: ls
monitor_cmd.sh README.md README.pdf
Success: Command executed successfully.
Enter a command: exit
Exiting command monitor

Hints
To execute a command, you can use the eval command followed by the user input. For example, eval ls
will execute the ls command. This is needed because the user input is a string, and you need to evaluate it
as a command.
```

### B1 solution (`solution_B1.sh`, from `onlineB1.zip`)

```bash
#!/usr/bin/bash

: '
The LED blinking will not work in WSL/VM as it does not have access to the hardware.
Only native Linux systems will be able to run this script successfully.
If you keyboard does not have a functional Caps Lock LED,
you can modify the script to use a different LED (numlock, scroll lock, etc.).

First grant execute permission to the script:
chmod +x solution_B1.sh

Then run the script:
sudo ./solution_B1.sh
'

# Change the LED_PATH based on your system
# Only the numeric part may vary, the rest should be the same
# For example: /sys/class/leds/input6::capslock/brightness
LED_PATH="/sys/class/leds/input24::capslock/brightness"

blink_led() {
  for i in {1..5}; do
    echo 1 >"$LED_PATH"
    sleep 0.1
    echo 0 >"$LED_PATH"
    sleep 0.1
  done
}

while true; do
  # Prompt the user for a command
  echo -n "Enter a command: "
  read user_command

  # If the user enters 'exit', break the loop and quit
  if [[ "$user_command" == "exit" ]]; then
    echo "Exiting command monitor."
    break
  fi

  # Run the user's command and capture the exit status
  eval "$user_command"
  status=$?

  # Check the exit status of the command
  if [[ $status -eq 0 ]]; then
    echo "Success: Command executed successfully."
  else
    echo "Failed: Command exited with status $status."
    blink_led
  fi
done
```

Key points:

- `eval` runs the input string as a command; `$?` decides success vs failure message.
- `exit` breaks the loop; anything else executes.
- Run with `sudo` (LED sysfs needs root); adjust only the numeric part of the LED path per machine.


---

<!-- ===== B2 — academic dirs ===== -->

# B2 — Academic Materials Directories (`2005ccc.sh`)

## Problem statement (from `b2files.zip` `onlineB2.pdf`)

```text
README.md

2024-09-18

Problem Description:
You are tasked with writing a shell script that reads an input file and creates directories based on the
contents. Each line in the file lists a course code along with the corresponding term. Your job is to organize
these courses into directories according to the following rules. Before you begin, install the tree package
to visualize the directory structure with this command: sudo apt install tree

Requirements:
Write a shell script that takes a file as input and creates directories based on the contents of the file. The
script should:
Accept a file as input. Show an error message if the file is not provided or does not exist.
Creates a main directory named Academic Materials.
Inside Academic Materials, creates directories for each term based on the course level and term
number, following these rules:
The first digit of the course code represents the level (e.g., "CSE 314" is level 3).
The term is represented by "T-NUMBER", where NUMBER is either 1 or 2.
The folder for each term should be named LXTY, where X is the level and Y is the term number.
If the course code is odd:
Create a directory inside the term folder named after the course code (e.g., CSE 301).
If the course code is even:
Create a directory inside the term folder called LABS.
Inside LABS, create a directory for each even-numbered course (e.g., CSE 314).
You should run the script like this: ./2005ccc.sh dirs.txt. You should check for the existence
of the file and show an error message if it does not exist or is not provided.

Example Input:
See the provided dirs.txt file.

Tips:
Use cut to extract columns or fields. For example, cut -d' ' -f1 will extract the first field from
each line, assuming that fields are separated by spaces. You can also extract multiple fields by
specifying a range, like this: cut -d' ' -f1-3.
You can also use cut to extract characters from a string. For example, cut -c1 will extract the first
character of the provided string.
Use --help or man to learn more about the commands you are using.
tree "Academic Materials" will display the directory structure under Academic Materials.
Important: See the next page for the expected directory structure
```

Expected directory structure (page 2):

```text
Academic Materials/
├── L3T2
│   ├── CSE 301
│   ├── CSE 313
│   ├── CSE 317
│   ├── CSE 321
│   ├── CSE 325
│   └── LABS
│       ├── CSE 314
│       ├── CSE 318
│       ├── CSE 322
│       └── CSE 326
├── L4T1
│   ├── CSE 405
│   ├── CSE 409
│   ├── CSE 423
│   ├── CSE 463
│   └── LABS
│       ├── CSE 406
│       └── CSE 408
└── L4T2
    ├── CSE 461
    ├── CSE 471
    ├── HUM 473
    ├── HUM 481
    ├── IPE 493
    └── LABS
        ├── CSE 462
        └── CSE 472
```

Sample `dirs.txt` lines:

```text
CSE 314 T-2
HUM 473 T-2
CSE 463 T-1
```

So per line: `dept(1) number(2) T-N(3)`. Level = first digit of the number; term dir = `L<level>T<N>`; odd number → `<term>/<dept number>`; even → `<term>/LABS/<dept number>`.

### B2 solution (`create_dirs.sh`, from `b2files.zip`)

```bash
#!/usr/bin/bash


if [ $# -ne 1 ]; then
    echo "Usage: ./create_academic_dirs.sh <input_file>"
    exit 1
fi

input_file=$1
main_dir="Academic Materials"
mkdir -p "$main_dir"

# Read the input file line by line
while read -r line; do
    # Extract course code, level and term
    course_code=$(echo "$line" | cut -d' ' -f1-2)
    level=$(echo "$line" | cut -d' ' -f2 | cut -c1)
    term=$(echo "$line" | cut -d' ' -f3 | cut -d'-' -f2)
    term_dir="L${level}T${term}"

    mkdir -p "$main_dir/$term_dir"

    # Extract the course number from the course code (second part of course code)
    course_number=$(echo "$line" | cut -d' ' -f2)

    if ((course_number % 2 == 1)); then
        mkdir -p "$main_dir/$term_dir/$course_code"
    else
        mkdir -p "$main_dir/$term_dir/LABS/$course_code"
    fi
done < "$input_file"

echo "Directory structure created successfully."
```

Key points:

- `cut -d' ' -f2 | cut -c1` takes the level digit; `cut -d' ' -f3 | cut -d'-' -f2` takes the term number after `T-`.
- Odd/even is decided on the course number (`% 2`); evens nest one level deeper under `LABS`.
- Quote `"$main_dir/$term_dir/..."` everywhere — names contain spaces (`CSE 314`).
