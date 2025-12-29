#include <stdio.h>

void toUpper(char *p) {
  while (*p != '\0') {
    if (*p >= 'a' && *p <= 'z')
      *p = *p - 32;
    p++;
  }
}

void toLower(char *p) {
  while (*p != '\0') {
    if (*p >= 'A' && *p <= 'Z')
      *p = *p + 32;
    p++;
  }
}

void toTitle(char *p) {
  int newWord = 1;

  while (*p != '\0') {
    if (*p == ' ') {
      newWord = 1;
    } else if (newWord && *p >= 'a' && *p <= 'z') {
      *p = *p - 32;
      newWord = 0;
    } else if (!newWord && *p >= 'A' && *p <= 'Z') {
      *p = *p + 32;
    } else {
      newWord = 0;
    }
    p++;
  }
}

int main() {
  char str1[] = "This is all an example using pointers i'm doing";
  char str2[] = "This is all an example using pointers i'm doing";
  char str3[] = "This is all an example using pointers i'm doing";

  toUpper(str1);
  toLower(str2);
  toTitle(str3);

  printf("Upper Case : %s\n", str1);
  printf("Lower Case : %s\n", str2);
  printf("Title Case : %s\n", str3);

  return 0;
}
