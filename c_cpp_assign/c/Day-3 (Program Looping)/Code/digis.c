#include <stdio.h>

int main() {
  int num;
  printf("Enter Num: ");
  scanf("%d", &num);

  int i = num;
  int count = 0;

  if (i == 0) {
    count = 1;
  } else {

    if (i < 0)
      i = -i;

    while (i > 0) {
      i = i / 10;
      count++;
    }
  }

  printf("Number of digits: %d\n", count);

  return 0;
}
