#include <stdio.h>

int main() {
  char names[5][50] = {"Vinayak", "Sudhakar", "Harsh", "Anushka", "Souvik"};

  char temp[50];
  int i, j, k;

  for (i = 0; i < 5 - 1; i++) {
    for (j = i + 1; j < 5; j++) {

      k = 0;
      while (names[i][k] == names[j][k] && names[i][k] != '\0')
        k++;

      if (names[i][k] > names[j][k]) {
        k = 0;
        while (names[i][k] != '\0' || names[j][k] != '\0') {
          temp[k] = names[i][k];
          names[i][k] = names[j][k];
          names[j][k] = temp[k];
          k++;
        }
        temp[k] = names[i][k];
        names[i][k] = names[j][k];
        names[j][k] = temp[k];
      }
    }
  }

  printf("Sorted Names:\n");
  for (i = 0; i < 5; i++)
    printf("%s\n", names[i]);

  return 0;
}
