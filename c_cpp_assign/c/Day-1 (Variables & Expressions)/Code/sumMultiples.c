// Purpose: Calculate sum of numbers below one million that are multiples of 35
// Date: 19/12/2025
// Author: Vinayak Majhi
// ID: 58618
// Batch ID: 25SUB4505 - DATACOM+5G

#include <stdio.h>

int main() {
    long long sum = 0;
    int i;

    for (i = 35; i < 1000000; i += 35) {
        sum = sum + i;
    }

    printf("Sum of all numbers below 1000000\n");
    printf("that are multiples of 5 and 7 = %lld\n", sum);

    return 0;
}
