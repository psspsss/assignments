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
