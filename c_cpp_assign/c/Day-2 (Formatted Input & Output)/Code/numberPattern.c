#include <stdio.h>

int main() {
    int n, i, j;

    printf("Accept a number: ");
    scanf("%d", &n);

    for (i = n; i <= n + 9; i++) {
        for (j = n; j <= n + 9; j++) {
            printf("%d\t", i * j);
        }
        printf("\n");
    }

    return 0;
}
