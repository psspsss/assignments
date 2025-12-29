// Purpose: Print the next 25 prime numbers after a given number
// Date: 22/12/2025
// Author: Vinayak Majhi
// ID: 58618
// Batch ID: 25SUB4505 - DATACOM+5G

#include <stdio.h>

int main() {
    int num, count = 0, i, isPrime;

    printf("Accept a number: ");
    scanf("%d", &num);

    num++; 

    while (count < 25) {
        isPrime = 1;

        for (i = 2; i * i <= num; i++) {
            if (num % i == 0) {
                isPrime = 0;
                break;
            }
        }

        if (isPrime && num > 1) {
            printf("%d ", num);
            count++;
        }

        num++;
    }

    return 0;
}
