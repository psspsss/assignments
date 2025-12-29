#include <stdio.h>

int main() {

  int n;
  printf("Enter a num: ");
  scanf("%d", &n);

  int a = 0, b = 1, next;
  int i = 0;
  while (i <= n) {
    printf("%d ", a);
    next = a + b;
    a = b;
    b = next;
    i++;
  }

  return 0;
}
