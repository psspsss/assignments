// Purpose: Illustrate casting between function pointer and integer and calling via cast (unsafe)
// Date: 24/12/2025
// Author: Vinayak Majhi
// ID: 58618
// Batch ID: 25SUB4505 - DATACOM+5G

#include <stdio.h>

int main() {
  long myInt = (long)printf;

  ((int (*)())myInt)("Hello World!...\n");
}
