#include <stdio.h>

int main() {
  int n = 0;
  printf("Enter a num: ");
  scanf("%d", &n);
  for (int i = 0; i < n; i++) {
    for (int j = 0; j <= i; j++) {

      printf("* ");
    }

    printf("\n");
  }

  printf("\n---------------\n");

  for (int a = 1; a <= n; a++) {
    for (int b = 1; b <= a; b++) {
      printf("%d ", b);
    }
    printf("\n");
  }

  printf("\n---------------\n");

  for (int a = n; a > 0; a--) {
    for (int b = a; b > 0; b--) {
      printf("* ");
    }
    printf("\n");
  }
  return 0;
}
