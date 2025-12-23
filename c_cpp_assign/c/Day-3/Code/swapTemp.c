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
