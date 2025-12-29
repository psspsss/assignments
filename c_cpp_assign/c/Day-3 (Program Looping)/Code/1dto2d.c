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
