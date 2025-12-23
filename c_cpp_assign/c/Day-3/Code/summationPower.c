#include <math.h>
#include <stdio.h>

int main() {

  int num;
  printf("Enter Num: ");
  scanf("%d", &num);

  if (num <= 0 || num > 30) {
    printf("Invalid");
    return 0;
  }

  long long sum = 1;

  for (int i = 1; i <= num; i++) {
    sum += (long long)pow(2, i);
  }

  printf("sum = %lld", sum);

  return 0;
}
