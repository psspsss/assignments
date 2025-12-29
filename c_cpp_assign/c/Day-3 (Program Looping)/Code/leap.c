// Purpose: Check whether a given year is a leap year using Gregorian rules
// Date: 23/12/2025
// Author: Vinayak Majhi
// ID: 58618
// Batch ID: 25SUB4505 - DATACOM+5G

#include <stdio.h>

int main() {

  int year;
  printf("Enter year: ");
  scanf("%d", &year);

  if (year % 400 == 0) {
    printf("Leap Year");
  }

  else if (year % 100 == 0) {
    printf("Not a leap year");
  }

  else if (year % 4 == 0) {
    printf("Leap Year");
  }

  else {
    printf("Not a Leap Year");
  }

  return 0;
}
