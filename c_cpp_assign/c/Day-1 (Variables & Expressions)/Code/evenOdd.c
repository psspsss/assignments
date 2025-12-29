// Purpose: Determine whether a given integer is even or odd
// Date: 19/12/2025
// Author: Vinayak Majhi
// ID: 58618
// Batch ID: 25SUB4505 - DATACOM+5G

#include <stdio.h>

int main() {
    int num;

    printf("Accept a number: ");
    scanf("%d", &num);

    if (num % 2 == 0)
        printf("%d is an Even number\n", num);
    else
        printf("%d is an Odd number\n", num);

    return 0;
}
