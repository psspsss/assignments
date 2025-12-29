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
