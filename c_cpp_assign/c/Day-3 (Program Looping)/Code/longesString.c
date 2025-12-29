// Purpose: Find and print the longest string from a fixed array of strings
// Date: 23/12/2025
// Author: Vinayak Majhi
// ID: 58618
// Batch ID: 25SUB4505 - DATACOM+5G

#include <stdio.h>

int main() {
  char str[5][100] = {"C programming", "Operating Systems", "Computer Networks",
                      "DBMS", "Data Structures and Algorithms"};

  int i, j, maxLen = 0, index = 0, len;

  for (i = 0; i < 5; i++) {
    len = 0;
    while (str[i][len] != '\0')
      len++;

    if (len > maxLen) {
      maxLen = len;
      index = i;
    }
  }

  printf("Longest String: %s\n", str[index]);
  printf("Length: %d\n", maxLen);

  return 0;
}
