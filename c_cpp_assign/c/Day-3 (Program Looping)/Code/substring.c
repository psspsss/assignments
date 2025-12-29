// Purpose: Find and replace occurrences of a substring within a string, then print
// Date: 23/12/2025
// Author: Vinayak Majhi
// ID: 58618
// Batch ID: 25SUB4505 - DATACOM+5G

#include <stdio.h>

void findAndReplace(char str[], char find[], char replace[]) {
  char result[300];
  int i = 0, k = 0;
  int j, match;
  int findLen = 0;

  while (find[findLen] != '\0')
    findLen++;

  while (str[i] != '\0') {
    match = 1;

    for (j = 0; j < findLen; j++) {
      if (str[i + j] != find[j]) {
        match = 0;
        break;
      }
    }

    if (match) {
      j = 0;
      while (replace[j] != '\0') {
        result[k++] = replace[j++];
      }
      i += findLen;
    } else {
      result[k++] = str[i++];
    }
  }

  result[k] = '\0';

  i = 0;
  while (result[i] != '\0') {
    str[i] = result[i];
    i++;
  }
  str[i] = '\0';
}

int main() {
  char str[300] = "This is a lengthy string example";
  char find[] = "lengthy";
  char replace[] = "short";

  findAndReplace(str, find, replace);

  printf("After Replacement: %s\n", str);

  return 0;
}
