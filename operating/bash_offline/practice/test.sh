#!/usr/bin/bash

name="sabbir"

var1=5
var2=89

echo "hello i am $name and my age is $var1 + $var2"

echo "hello i am $name and my age is $((var1 + var2))"

sum=$((var1 + var2))

echo "printing $sum"

if [ $sum -gt 100 ]; then
  echo " greaeer than 2 "

else
  echo " not man grateaer "
fi
echo " done till now . next to learn : loops  "

numarr=(1 23 2 34 4 5 7 9 10)

fruits=("apple" "bannana" "peach ")

colors[0]="red"
colors[1]="blue"

sum=0

for num in "${numarr[@]}"; do
  if ((num % 2 != 0)); then
    ((sum += num))

  fi

done

echo "the sum if $sum"

read -p "enter array size " size

echo "inout number $size times "

arr=()

for ((i = 0; i < $size; i++)); do
  read -p "enter value $((i)) : " val
  arr[$i]=$val
done

for int in "${arr[@]}"; do
  echo "$int"

done
