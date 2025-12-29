// Purpose: Populate and print an array of 100 sequential integers starting from input
// Date: 22/12/2025
// Author: Vinayak Majhi
// ID: 58618
// Batch ID: 25SUB4505 - DATACOM+5G

#include <stdio.h>

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

    return 0;
}
