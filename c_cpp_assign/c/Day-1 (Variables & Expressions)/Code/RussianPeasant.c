// Purpose: Multiply two numbers using Russian peasant multiplication method
// Date: 19/12/2025
// Author: Vinayak Majhi
// ID: 58618
// Batch ID: 25SUB4505 - DATACOM+5G

#include <stdio.h>

int main() {
    int first, second;
    int result = 0;

    printf("Enter two numbers: ");
    scanf("%d %d", &first, &second);

    printf("\nfirst\tsecond\t\t--> result\n");

    while (first > 0) {
        printf("%d\t%d\t\t--> ", first, second);

        if (first % 2 != 0) {
            result = result + second;
            printf("%d\n", result);
        } else {
            printf("%d\n", result);
        }

        first = first / 2;
        second = second * 2;
    }

    printf("\nPrint res --> %d\n", result);

    return 0;
}
