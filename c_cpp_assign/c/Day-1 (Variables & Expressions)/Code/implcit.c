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
