// Purpose: Perform chosen arithmetic operation using if-else based calculator
// Date: 22/12/2025
// Author: Vinayak Majhi
// ID: 58618
// Batch ID: 25SUB4505 - DATACOM+5G

#include <stdio.h>

int main() {
    int a, b;
    char op;

    printf("Enter two numbers: ");
    scanf("%d %d", &a, &b);

    printf("Enter operation (+ - * /): ");
    scanf(" %c", &op);

    if (op == '+')
        printf("Result: %d\n", a + b);
    else if (op == '-')
        printf("Result: %d\n", a - b);
    else if (op == '*')
        printf("Result: %d\n", a * b);
    else if (op == '/')
        printf("Result: %d\n", a / b);
    else
        printf("Invalid operation\n");

    return 0;
}
