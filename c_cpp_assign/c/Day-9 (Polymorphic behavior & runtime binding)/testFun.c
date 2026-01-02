#include <stdio.h>

void fun();
void funOne(int);
void funTwo(int, int);
void funThree(int, int, int);
void funFour();

int main() {
  fun();
  funOne(10);
  funTwo(100, 200);
  funThree(11, 22, 33);
  funFour();
}
void fun() { printf("void fun() called...\n"); }
void funOne(int x) { printf("void funOne(%d) called...\n", x); }
void funTwo(int x, int y) { printf("void funTwo(%d, %d) called...\n", x, y); }
void funThree(int x, int y, int z) {
  printf("void funThree(%d, %d, %d) called...\n", x, y, z);
}
void funFour() { printf("void funFour() called...\n"); }
