#! /bin/bash
b=
c=""
d="\"\""
if [ "$a" == "$b" ]; then echo "a equals b"; fi
if [ "$b" == "$c" ]; then echo "b equals c"; fi
if [ "$c" == "$d" ]; then echo "c equals d"; fi
if [ -z "$a" ]; then echo "a is empty"; fi
if [ -z "$b" ]; then echo "b is empty"; fi
if [ -z "$c" ]; then echo "c is empty"; fi
if [ -z "$d" ]; then echo "d is empty"; fi

export aa
export bb=$b
export cc=$c
export dd=$d

echo "a=$a"
echo "b=$b"
echo "c=$c"
echo "d=$d"

echo "aa=$aa"
echo "bb=$bb"
echo "cc=$cc"
echo "dd=$dd"