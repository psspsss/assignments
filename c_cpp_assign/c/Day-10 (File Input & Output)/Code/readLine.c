#include <stdio.h>

int main(int argc, char *argv[]) {
  printf("Count of args: %d\n", argc);
  printf("Argument List: \n");
  for (int cnt = 0; cnt < argc; cnt++)
    printf("\t\targv[%d]  ==> %s\n", cnt, argv[cnt]);
}
