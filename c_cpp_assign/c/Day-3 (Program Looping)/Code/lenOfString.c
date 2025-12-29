// Purpose: Count characters, words, and spaces in a string using array indexing
// Date: 23/12/2025
// Author: Vinayak Majhi
// ID: 58618
// Batch ID: 25SUB4505 - DATACOM+5G

#include <stdio.h>

int main() {
  char str[] = "This is using array indexing";
  int i = 0;
  int words = 0, chars = 0, spaces = 0;

  while (str[i] != '\0') {
    chars++;

    if (str[i] == ' ')
      spaces++;

    if ((i == 0 && str[i] != ' ') || (str[i] != ' ' && str[i - 1] == ' '))
      words++;

    i++;
  }

  printf("String: %s\n", str);
  printf("Characters: %d\n", chars);
  printf("Spaces: %d\n", spaces);
  printf("Words: %d\n", words);

  return 0;
}
