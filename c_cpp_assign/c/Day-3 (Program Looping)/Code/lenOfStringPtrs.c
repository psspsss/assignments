// Purpose: Count characters, words, and spaces in a string using pointer traversal
// Date: 23/12/2025
// Author: Vinayak Majhi
// ID: 58618
// Batch ID: 25SUB4505 - DATACOM+5G

#include <stdio.h>

int main() {
  char str[] = "this was done using pointers";
  char *p = str;

  int words = 0, chars = 0, spaces = 0;

  while (*p != '\0') {
    chars++;

    if (*p == ' ')
      spaces++;

    if ((p == str && *p != ' ') || (*p != ' ' && *(p - 1) == ' '))
      words++;

    p++;
  }

  printf("String: %s\n", str);
  printf("Characters: %d\n", chars);
  printf("Spaces: %d\n", spaces);
  printf("Words: %d\n", words);

  return 0;
}
