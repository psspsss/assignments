#include <stdio.h>

int main() {
  long myInt = (long)printf;

  ((int (*)())myInt)("Hello World!...\n");
}
