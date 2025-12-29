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
