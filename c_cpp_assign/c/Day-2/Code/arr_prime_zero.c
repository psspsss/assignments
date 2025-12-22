#include <stdio.h>

int isPrime(int num) {
    int i;

    if (num <= 1)
        return 0;

    for (i = 2; i * i <= num; i++) {
        if (num % i == 0)
            return 0;
    }
    return 1;
}

int main() {
    int arr[100];
    int i;

    printf("Enter the first value: ");
    scanf("%d", &arr[0]);

    for (i = 1; i < 100; i++) {
        arr[i] = arr[i - 1] + 1;
    }

    for (i = 0; i < 100; i++) {
        printf("%d ", arr[i]);
    }

    printf("\n");


    for (i = 0; i < 100; i++) {
        if (!isPrime(arr[i])) {
            arr[i] = 0;
        }
    }


    for (i = 0; i < 100; i++) {
        printf("%d ", arr[i]);
    }

    return 0;
}
