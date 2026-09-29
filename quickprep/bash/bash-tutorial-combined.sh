#!/bin/bash
# Combined from Bash Script Tutorial/t/ (18 files)
# Sources: 00_intro.sh, 01_variable.sh, 02_command_substitution.sh, 03_expression.sh, 04_condition.sh, 05_logicop.sh, 06_ifcond.sh, 07_case.sh, 08_while_until.sh, 09_brace_expansion.sh, 10_for.sh, 11_array.sh, 12_map.sh, 13_stdin.sh, 14_ifs.sh, 15_file.sh, 16_cmd_args.sh, 17_function.sh


# ================= 00_intro.sh =================
#!/usr/bin/bash

# This is a comment

echo "Hello world" # It can be inline

echo -e "print(\"hello world\")\nexit(5)" > main.py
python3 main.py

echo $?

rm main.py

(cd /)
ls

exit 41


# ================= 01_variable.sh =================
#!/usr/bin/bash

greet="Greetings "
greet="$greet from"

name=Imtiaz
name+=Kabir

echo "$greet"
echo $name

greetname=0123
echo $greetname
echo ${greet}name
echo $greet$name


# ================= 02_command_substitution.sh =================
#!/usr/bin/bash

id=$(echo 041)
name=$(whoami)

echo "$id $name"

err=$(gcc)
echo $err


# ================= 03_expression.sh =================
#!/usr/bin/bash

expr 7 + 2
expr 7 - 2
expr 7 \* 2
expr 7 / 2

n=$(expr 7 + 2)
echo $n

n=$((7 * 2))
echo $n

((n = 7 / 2))
echo $n


# ================= 04_condition.sh =================
#!/usr/bin/bash

true
echo $?

false
echo $?

echo "Hello world"
echo $?

(exit 41)
echo $?

test 5 -gt 1
echo $?

test 10 -lt 3
echo $?

test 10 -eq 10
echo $?

[ 5 -gt 1 ]
echo $?

((100 > 1))
echo $?

((100 < 1))
echo $?


# ================= 05_logicop.sh =================
#!/usr/bin/bash

true && false
echo $?

false || true
echo $?

echo "Bash" && echo "scripting"
echo $?

echo "Hello" || echo "World"
echo $?

! echo "hi"
echo $?

n=4

test $n -gt 1 && test $n -lt 10
echo $?

[ $n -gt 1 ] && [ $n -lt 10 ]
echo $?


(( $n > 1 && $n < 10 ))
echo $?


# ================= 06_ifcond.sh =================
#!/usr/bin/bash

if true
then
  echo "true condition"
fi

if true; then
  echo "true again"
fi

if false; then
  echo "You cant see me"
fi

if ! echo "Hello"; then
  echo "World"
fi


n=5
if test $n -eq 5; then
  echo "n is 5"
fi

if [ $n -lt 10 ]; then
  echo "n is less than 10"
elif [ $n -eq 10 ]; then
  echo "n is 10"
else
  echo "n is more than 10"
fi

if [ $(expr $n + 2) -eq 7 ]; then
  echo "n + 2 is 7"
fi


if test $n -gt 2 && test $n -lt 8; then
  echo "2 < n < 8"
fi

if [ $n -gt 2 ] && [ $n -lt 8 ]; then
  echo "2 < n < 8"
fi


if [[ $n > 2 ]] && [[ $n < 8 ]]; then
  echo "2 < n < 8"
fi

if [[ 2 < $n && $n < 8 ]]; then
  echo "2 < n < 8"
fi


if [[ 2 -le $n && $n -le 8 ]]; then
  echo "2 <= n <= 8"
fi


if (( 2 <= $n && $n <= 8 )); then
  echo "2 <= n <= 8"
fi


# ================= 07_case.sh =================
#!/usr/bin/bash


day=$(date +%w)

case $day in
  0)
    echo "Sunday"
    ;;
  1)
    echo "Monday"
    ;;
  2)
    echo "Tuesday"
    ;;
  3)
    echo "Wednesday"
    ;;
  4)
    echo "Thursday"
    ;;
  5)
    echo "Friday"
    ;;
  6)
    echo "Saturday"
    ;;
esac



case $day in
  4|5) echo "Yay! No BUET";;
  *) echo "All work and no play";;
esac


filename="backup_2026.tar.gz"

case "$filename" in
    *.jpg|*.png|*.gif) echo "Image";;
    *.tar.gz|*.tgz|*.zip) echo "Compressed archive";;
    report_??.txt) echo "Report_XY";;
    [a-z]*[0-9]) echo "Lowercase #### digit";;
    *) echo "Unknown file format.";;
esac


# ================= 08_while_until.sh =================
#!/usr/bin/bash

i=0
while (($i < 5)); do
  echo $i
  ((i++))
  # continue
  # echo "Hello"
done

i=0
until (($i >= 5)); do
  echo $i
  ((i++))
done


# ================= 09_brace_expansion.sh =================
#!/usr/bin/bash

echo {1..5}

echo {5..1}

echo {0..20..5}

echo {a..f}

echo file{1..5}.txt

echo {cat,dog,bird}

echo CSE_{A,B,C}{1,2}

echo {x,{a,b}}{1,2}


n=5
echo {1, $n} # Does not work
seq 1 $n


# ================= 10_for.sh =================
#!/usr/bin/bash

for i in bash is easy
do
  echo $i
done

for i in {1..5}; do
  echo $i
done

for i in $(ls); do
  echo $i
done

for i in $(seq 1 2 6); do
  echo $i
done

sum=0
for ((i = 1; i <= 1000; i++)); do
  ((sum += i))
done

echo $sum


# ================= 11_array.sh =================
#!/usr/bin/bash

languages=(c cpp java python js)

echo "languages =" ${languages[@]}
echo "languages[1] =" ${languages[1]}
echo "len(languages) =" ${#languages[@]}

# Append
languages+=(zig rust)
echo "After insertion" "${languages[@]}"

# Deletion
unset languages[1]
echo "After deletion" "${languages[@]}"


# ================= 12_map.sh =================
#!/usr/bin/bash

declare -A capital=(
  [Bangladesh]=Dhaka
  [China]=Beijing
  [Russia]=Moscow
)

echo "Keys =" "${!capital[@]}"
echo "Values =" "${capital[@]}"


if [[ ! -v capital[France] ]]; then
  echo "France not in key set of capital"
  capital[France]=Paris
  echo "${capital[France]}"
fi

unset capital[China]
echo "Keys =" "${!capital[@]}"
echo "Values =" "${capital[@]}"


# ================= 13_stdin.sh =================
#!/usr/bin/bash

echo -n "Enter your name: "
read name
echo "Hello, $name"


# ================= 14_ifs.sh =================
#!/usr/bin/bash

data="Apple,Banana,Orange"

OLD_IFS="$IFS"
IFS=","
for fruit in $data; do
    echo "Fruit: $fruit"
done
IFS="$OLD_IFS"

IFS="," read fruit1 fruit2 fruit3 <<< "$data"
echo $fruit3


# ================= 15_file.sh =================
#!/usr/bin/bash


while IFS= read -r line; do
  echo "LINE: $line"
done < "readme.txt"

content=$(<"readme.txt")
echo "$content"

mapfile lines < "readme.txt"
echo "L0: ${lines[0]}"


# ================= 16_cmd_args.sh =================
#!/usr/bin/bash

echo "The script name is: $0"
echo "The first argument is: ${1}"
echo "The second argument is: ${2:-default2}"
echo "The total number of arguments passed is: $#"

echo "Arguments:" "$@"


# ================= 17_function.sh =================
#!/usr/bin/bash

foo() {
  echo "Hello world"
  echo "Bye"
  return 41
}

msg=$(foo)
echo $?
echo "$msg"


bar() {
  echo "The first argument is: ${1}"
  echo "The second argument is: ${2:-default2}"
  echo "The total number of arguments passed is: $#"

  echo "Arguments:" "$@"
}

bar hello world of bash
