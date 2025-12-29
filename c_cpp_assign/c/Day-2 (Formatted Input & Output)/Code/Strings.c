#include <stdio.h>


int myStrLen(const char str[]) {
    int i = 0;

    while (str[i] != '\0') {
        i++;
    }
    return i;
}

char *myStrCpy(char dest[], const char src[]) {
    int i = 0;

    while (src[i] != '\0') {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';

    return dest;
}


char *myStrCat(char dest[], const char src[]) {
    int i = 0, j = 0;

    while (dest[i] != '\0') {
        i++;
    }

    while (src[j] != '\0') {
        dest[i] = src[j];
        i++;
        j++;
    }
    dest[i] = '\0';

    return dest;
}

int myStrCmp(const char str1[], const char str2[]) {
    int i = 0;

    while (str1[i] != '\0' && str2[i] != '\0') {
        if (str1[i] != str2[i])
            return str1[i] - str2[i];
        i++;
    }

    return str1[i] - str2[i];
}

int main() {
    char s1[100] = "Vinayak";
    char s2[100] = "Majhi";
    char s3[100];

    printf("Length of s1: %d\n", myStrLen(s1));

    myStrCpy(s3, s1);
    printf("After copy, s3: %s\n", s3);

    myStrCat(s3, s2);
    printf("After concat, s3: %s\n", s3);

    printf("Compare s1 and s2: %d\n", myStrCmp(s1, s2));
    printf("Compare s1 and s1: %d\n", myStrCmp(s1, s1));

    return 0;
}
