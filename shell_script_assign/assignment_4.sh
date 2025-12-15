#!/bin/bash

mkdir TestDir

cd TestDir || exit 1

for i in {1..10}; do
  filename="File$i.txt"
  echo "$filename" >"$filename"
done

echo "TestDir directory created with 10 files"
