#!/bin/bash

LOGFILE="sample_log.log"

if [ ! -f "$LOGFILE" ]; then
  echo "Error: Log file not found."
  exit 1
fi

grep "ERROR" "$LOGFILE" |
  sed 's/ERROR //' |
  awk '{print "Date:", $1, "| Time:", $2, "| Message:", substr($0, index($0,$3))}'
