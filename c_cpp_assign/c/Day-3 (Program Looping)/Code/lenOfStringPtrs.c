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
