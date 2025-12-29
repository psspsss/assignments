// Purpose: Reverse digits of an integer and display the reversed number
// Date: 23/12/2025
// Author: Vinayak Majhi
// ID: 58618
// Batch ID: 25SUB4505 - DATACOM+5G

#include <stdio.h>

int main() {

  int num;
  printf("Enter num: ");
  scanf("%d", &num);

  int rev = 0;

  while (num > 0) {
    int digit = num % 10;
    rev = (rev * 10) + digit;
    num = num / 10;
  }
  printf("%d", rev);

  return 0;
}
