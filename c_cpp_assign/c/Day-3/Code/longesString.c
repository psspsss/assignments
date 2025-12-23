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
