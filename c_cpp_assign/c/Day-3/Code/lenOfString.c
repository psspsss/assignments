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
