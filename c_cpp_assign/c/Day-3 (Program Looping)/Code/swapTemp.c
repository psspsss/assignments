// Purpose: Swap values of two variables using a temporary third variable
// Date: 23/12/2025
// Author: Vinayak Majhi
// ID: 58618
// Batch ID: 25SUB4505 - DATACOM+5G

#include <stdio.h>

int main() {

  int a, b, c;

  printf("Enter 2 Nums: ");
  scanf("%d %d", &a, &b);

  printf("a = %d\n", a);
  printf("b = %d\n", b);

  c = a;
  a = b;
  b = c;

  printf("a = %d\n", a);
  printf("b = %d\n", b);

  return 0;
}
