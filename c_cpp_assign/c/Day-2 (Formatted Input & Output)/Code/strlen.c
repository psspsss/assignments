// Purpose: Calculate and print string length using custom function myStrLen
// Date: 22/12/2025
// Author: Vinayak Majhi
// ID: 58618
// Batch ID: 25SUB4505 - DATACOM+5G

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
