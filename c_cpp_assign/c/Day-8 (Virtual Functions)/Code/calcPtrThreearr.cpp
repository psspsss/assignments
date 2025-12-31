#include <stdio.h>

int Add(int, int);
int Sub(int, int);
int Divi(int, int);
int Mult(int, int);
int Modi(int, int);

typedef int (*FPTR)(int, int);

int calc(FPTR, int, int);

int main() {
  FPTR arr[] = {Add, Sub, Mult, Divi, Modi, Add, Add, Sub, Mult, NULL};

  for (int cnt = 0; arr[cnt] != NULL; cnt++)
    printf("calculating... %d\n", calc(arr[cnt], 100, 20));
}

int Add(int x, int y) { return x + y; }
int Sub(int x, int y) { return x - y; }
int Divi(int x, int y) { return x / y; }
int Mult(int x, int y) { return x * y; }
int Modi(int x, int y) { return x % y; }
int calc(FPTR fPtr, int x, int y) { return fPtr(x, y); }
