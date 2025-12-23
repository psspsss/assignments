
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
