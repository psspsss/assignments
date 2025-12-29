#include <stdio.h>

int main() {
    int a = 8, b = 4;

    printf("(a + b) * 2 = %d\n", (a + b) * 2);
    printf("a > b = %d\n", a > b);
    printf("a == b = %d\n", a == b);
    printf("(a > b) && (b > 0) = %d\n", (a > b) && (b > 0));

    return 0;
}
