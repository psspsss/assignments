// Purpose: Determine and print the largest of three input integers
// Date: 23/12/2025
// Author: Vinayak Majhi
// ID: 58618
// Batch ID: 25SUB4505 - DATACOM+5G

#include <stdio.h>

int main() {

  int a, b, c;

  printf("Enter 3 Numbers: ");
  scanf("%d %d %d", &a, &b, &c);

  if (a >= b && a >= c) {
    printf("%d is the greatest", a);
  }

  else if (b >= a && b >= c) {
    printf("%d is the greatest", b);
  }

  else {
    printf("%d is the greatest", c);
  }

  return 0;
}
