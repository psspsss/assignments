#include <stdio.h>

int main() {
    int i;

    printf("For loop: ");
    for (i = 1; i <= 100; i++) {
        printf("%d ", i);
    }
    printf("\nWhile loop: ");
    i = 1;  
    while (i <= 100) {
        printf("%d ", i);
        i++;
    }

    printf("\nDo-while loop: ");
    i = 1; 
    do {
        printf("%d ", i);
        i++;
    } while (i <= 100);

    return 0;
}
