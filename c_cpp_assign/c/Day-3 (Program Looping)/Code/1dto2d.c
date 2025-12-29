// Purpose: Split a sentence into words and store them in a 2D array, then print
// Date: 23/12/2025
// Author: Vinayak Majhi
// ID: 58618
// Batch ID: 25SUB4505 - DATACOM+5G

#include <stdio.h>

int main() {
  char str[200] = "C programming is very easy to learn";
  char words[20][20];
  int i = 0, row = 0, col = 0;

  while (str[i] != '\0') {
    if (str[i] == ' ') {
      words[row][col] = '\0';
      row++;
      col = 0;
    } else {
      words[row][col] = str[i];
      col++;
    }
    i++;
  }
  words[row][col] = '\0';

  printf("Words stored in 2D array:\n");
  for (i = 0; i <= row; i++)
    printf("%s\n", words[i]);

  return 0;
}
