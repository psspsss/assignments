// Purpose: Check whether an integer is prime using trial division up to sqrt(n)
// Date: 19/12/2025
// Author: Vinayak Majhi
// ID: 58618
// Batch ID: 25SUB4505 - DATACOM+5G

#include <stdio.h>

int main() {
    int num, i, isPrime = 1;

    printf("Enter a num: ");
    scanf("%d", &num);

    if (num <= 1) {
        isPrime = 0;
    } else {
        for (i = 2; i * i <= num; i++) {
            if (num % i == 0) {
                isPrime = 0;
                break;
            }
        }
    }

    if (isPrime)
        printf("%d is a prime\n", num);
    else
        printf("%d is NOT a prime\n", num);

    return 0;
}
