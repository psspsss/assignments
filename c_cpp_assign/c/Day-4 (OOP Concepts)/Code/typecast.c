#include <stdio.h>

int main() {
  int a = 10;
  float b = 3.14f;
  char c = 'X';

  int *ip = &a;
  float *fp = &b;
  char *cp = &c;

  fp = ip;
  cp = ip;
  ip = cp;

  printf("Addresses:\n");
  printf("%p\n", ip);
  printf("%p\n", fp);
  printf("%p\n", cp);

  return 0;
}
