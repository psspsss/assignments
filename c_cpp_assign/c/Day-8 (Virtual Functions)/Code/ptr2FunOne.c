#include <stdio.h>

void fun();

int main() {
  void (*funPtr)(); // declaration of a pointer to function taking no args
                    // return nothing

  funPtr = &fun; // funPtr = fun

  funPtr(); // calling fun() using pointer
}

void fun() { printf("fun() called\n"); }
