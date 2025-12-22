#include <stdio.h>

int myStrLen(const char str[]) {
    int i = 0;
    while (str[i] != '\0')
        i++;
    return i;
}

int main() {
    char str[100];

    printf("Enter string: ");
    scanf("%s", str);

    printf("Length = %d\n", myStrLen(str));

    return 0;
}
