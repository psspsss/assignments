// Purpose: Show implicit type conversion between int, float, and double during arithmetic
// Date: 19/12/2025
// Author: Vinayak Majhi
// ID: 58618
// Batch ID: 25SUB4505 - DATACOM+5G

#include <stdio.h>

int main() {
    int a = 10;
    float b = 3.5;
    double c = 2.25;

    printf("int + float = %.2f\n", a + b);
    printf("float + double = %.2f\n", b + c);
    printf("int + double = %.2f\n", a + c);

    return 0;
}
