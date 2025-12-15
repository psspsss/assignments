#!/bin/bash

while true; do
  echo "Enter any number (type 0 to exit)"

  read num

  if [ "$num" -eq 0 ]; then
    echo "Process exited by the user"
    break

  fi

  if [ $((num % 2)) -eq 0 ]; then
    echo "$num is even"

  else
    echo "$num is odd"

  fi

done
