// Purpose: Reverse a string in-place and print the result
// Date: 22/12/2025
// Author: Vinayak Majhi
// ID: 58618
// Batch ID: 25SUB4505 - DATACOM+5G

#include <stdio.h>

void reverse(char str[]) {
    int i = 0, j;
    char temp;

    while (str[i] != '\0')
        i++;
    j = i - 1;

    i = 0;
    while (i < j) {
        temp = str[i];
        str[i] = str[j];
        str[j] = temp;
        i++;
        j--;
    }
}

int main() {
    char str[100];

    printf("Enter string: ");
    scanf("%s", str);

    reverse(str);
    printf("Reversed string: %s\n", str);

    return 0;
}
