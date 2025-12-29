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
