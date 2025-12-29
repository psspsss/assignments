// Purpose: Print multiplication table block (10x10) starting from input number
// Date: 22/12/2025
// Author: Vinayak Majhi
// ID: 58618
// Batch ID: 25SUB4505 - DATACOM+5G

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
