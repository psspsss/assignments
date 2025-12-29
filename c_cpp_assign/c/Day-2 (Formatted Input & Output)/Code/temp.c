// Purpose: Convert temperature between Celsius and Fahrenheit using simple functions
// Date: 22/12/2025
// Author: Vinayak Majhi
// ID: 58618
// Batch ID: 25SUB4505 - DATACOM+5G

#include <stdio.h>

float cToF(float c) {
    return (c * 9 / 5) + 32;
}

float fToC(float f) {
    return (f - 32) * 5 / 9;
}

int main() {
    float temp;
    int choice;

    printf("1.C to F\n2.F to C\nEnter choice: ");
    scanf("%d", &choice);

    printf("Enter temperature: ");
    scanf("%f", &temp);

    if (choice == 1)
        printf("Result: %.2f F\n", cToF(temp));
    else
        printf("Result: %.2f C\n", fToC(temp));

    return 0;
}
