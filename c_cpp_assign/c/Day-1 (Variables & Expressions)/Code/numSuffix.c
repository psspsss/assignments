// Purpose: Print ordinal suffix (st, nd, rd, th) for an integer
// Date: 19/12/2025
// Author: Vinayak Majhi
// ID: 58618
// Batch ID: 25SUB4505 - DATACOM+5G

#include <stdio.h>

int main() {
    int num;
    int lastDigit, lastTwoDigits;

    printf("Enter the number : ");
    scanf("%d", &num);

    lastDigit = num % 10;
    lastTwoDigits = num % 100;

    if (lastTwoDigits >= 11 && lastTwoDigits <= 13)
        printf("Output: %dth\n", num);
    else if (lastDigit == 1)
        printf("Output: %dst\n", num);
    else if (lastDigit == 2)
        printf("Output: %dnd\n", num);
    else if (lastDigit == 3)
        printf("Output: %drd\n", num);
    else
        printf("Output: %dth\n", num);

    return 0;
}
