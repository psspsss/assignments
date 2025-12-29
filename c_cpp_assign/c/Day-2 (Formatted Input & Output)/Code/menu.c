// Purpose: Simple menu to perform add, subtract, or multiply based on choice
// Date: 22/12/2025
// Author: Vinayak Majhi
// ID: 58618
// Batch ID: 25SUB4505 - DATACOM+5G

#include <stdio.h>

int main() {
    int choice, a = 10, b = 5;

    printf("1.Add\n2.Subtract\n3.Multiply\nEnter choice: ");
    scanf("%d", &choice);

    if (choice >= 1 && choice <= 3) {
        switch (choice) {
            case 1: printf("Sum = %d\n", a + b); break;
            case 2: printf("Difference = %d\n", a - b); break;
            case 3: printf("Product = %d\n", a * b); break;
        }
    } else {
        printf("Invalid choice\n");
    }

    return 0;
}
