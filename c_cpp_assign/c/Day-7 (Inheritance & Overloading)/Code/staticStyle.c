#include <stdio.h>
#include <string.h>

int main() {
  char str[] = "One way of using a class is by creating objects and using the "
               "member through that object";
  char *ptr = strtok(str, " "); // FIRST time giving address
  while (ptr != NULL) {
    printf("%s\n", ptr);
    ptr = strtok(NULL, " "); // consecutive no address given
  }
}
