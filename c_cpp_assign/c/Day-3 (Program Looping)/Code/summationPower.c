// Purpose: Compute sum of powers of two from 2^0 to 2^n using pow function
// Date: 23/12/2025
// Author: Vinayak Majhi
// ID: 58618
// Batch ID: 25SUB4505 - DATACOM+5G

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
