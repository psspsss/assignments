// Purpose: Compute sum of first n natural numbers using a loop
// Date: 23/12/2025
// Author: Vinayak Majhi
// ID: 58618
// Batch ID: 25SUB4505 - DATACOM+5G


#include <stdio.h>

int main() {

  int num;
  printf("Enter a num: ");
  scanf("%d", &num);

  int a = 0;
  for (int i = 1; i <= num; i++) {

    a += i;
  }

  printf("%d", a);
  return 0;
}
