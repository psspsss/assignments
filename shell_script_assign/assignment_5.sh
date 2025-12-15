#!/bin/bash

DEBUG=false

if [ "$1" == "-d" ]; then
  DEBUG=true
  shift
fi

log_debug() {
  if [ "$DEBUG" = true ]; then
    echo "[DEBUG] $1"
  fi
}

DIR="TestDir"

if [ -d "$DIR" ]; then
  echo "Error: Directory '$DIR' already exists."
  exit 1
fi

log_debug "Creating directory $DIR"
mkdir "$DIR" || {
  echo "Error: Failed to create directory '$DIR' (permission denied?)."
  exit 1
}

cd "$DIR" || {
  echo "Error: Cannot enter directory '$DIR'."
  exit 1
}

log_debug "Inside directory $(pwd)"

for i in {1..10}; do
  filename="File$i.txt"
  log_debug "Creating file $filename"

  echo "$filename" >"$filename" || {
    echo "Error: Failed to create $filename."
    exit 1
  }
done

echo "TestDir directory created successfully with 10 files."
