#!/bin/bash

count_lines() {
  filename="$1"

  if [ -f "$filename" ]; then
    lines=$(wc -l <"$filename")
    echo "File: $filename → $lines lines"
  else
    echo "File: $filename → File not found"
  fi
}

for file in "$@"; do
  count_lines "$file"
done
