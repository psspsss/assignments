#include <stdio.h>

int Add(int, int);

int Sub(int, int);

int Divi(int, int);

int Mult(int, int);

int Modi(int, int);

int calc(int (*)(int, int), int, int);

int main() {

  printf("Adding %d\n", calc(Add, 100, 20));

  printf("Subtracting %d\n", calc(Sub, 100, 20));

  printf("Dividing %d\n", calc(Divi, 100, 20));

  printf("Multiplying %d\n", calc(Mult, 100, 20));

  printf("Modulus %d\n", calc(Modi, 100, 3));
}

int Add(int x, int y) { return x + y; }

int Sub(int x, int y) { return x - y; }

int Divi(int x, int y) { return x / y; }

int Mult(int x, int y) { return x * y; }

int Modi(int x, int y) { return x % y; }

int calc(int (*fPtr)(int, int), int x, int y) { return fPtr(x, y); }
