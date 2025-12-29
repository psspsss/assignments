// Purpose: Convert strings to upper, lower, and title case using array indexing
// Date: 23/12/2025
// Author: Vinayak Majhi
// ID: 58618
// Batch ID: 25SUB4505 - DATACOM+5G

#include <stdio.h>

void toUpper(char str[]) {
  int i = 0;
  while (str[i] != '\0') {
    if (str[i] >= 'a' && str[i] <= 'z')
      str[i] = str[i] - 32;
    i++;
  }
}

void toLower(char str[]) {
  int i = 0;
  while (str[i] != '\0') {
    if (str[i] >= 'A' && str[i] <= 'Z')
      str[i] = str[i] + 32;
    i++;
  }
}

void toTitle(char str[]) {
  int i = 0;
  int newWord = 1;

  while (str[i] != '\0') {
    if (str[i] == ' ') {
      newWord = 1;
    } else if (newWord && str[i] >= 'a' && str[i] <= 'z') {
      str[i] = str[i] - 32;
      newWord = 0;
    } else if (!newWord && str[i] >= 'A' && str[i] <= 'Z') {
      str[i] = str[i] + 32;
    } else {
      newWord = 0;
    }
    i++;
  }
}

int main() {
  char str1[] = "This is all an example using array indexing i'm doing";
  char str2[] = "This is all an example using array indexing i'm doing";
  char str3[] = "This is all an example using array indexing i'm doing";

  toUpper(str1);
  toLower(str2);
  toTitle(str3);

  printf("Upper Case : %s\n", str1);
  printf("Lower Case : %s\n", str2);
  printf("Title Case : %s\n", str3);

  return 0;
}
