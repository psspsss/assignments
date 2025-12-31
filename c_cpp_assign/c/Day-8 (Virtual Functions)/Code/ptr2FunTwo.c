#include <stdio.h>

void fun();

void funCaller(void (*)());

int main() {
  funCaller(fun); // funCaller(&fun);
}

void fun() { printf("fun() called\n"); }

void funCaller(void (*fPtr)()) { fPtr(); }
