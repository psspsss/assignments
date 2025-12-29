// Purpose: Calculate simple interest and final amount from principal, rate, and time
// Date: 23/12/2025
// Author: Vinayak Majhi
// ID: 58618
// Batch ID: 25SUB4505 - DATACOM+5G

#include <stdio.h>

int main() {

  int p, t;
  float r;
  printf("Enter Principal: ");
  scanf("%d", &p);

  printf("Enter Rate: ");
  scanf("%f", &r);

  printf("Enter Time Period: ");
  scanf("%d", &t);
  t = t / 12.0;

  float interest = ((p * r * t) / 100);

  float amount = p + interest;

  printf("Amount = %f", amount);

  return 0;
}
