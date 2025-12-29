#include <stdio.h>

int main() {
    int age;
    float height;
    char grade;
    char name[50];

    printf("Enter name: ");
    scanf("%s", name);

    printf("Enter age: ");
    scanf("%d", &age);

    printf("Enter height: ");
    scanf("%f", &height);

    printf("Enter grade: ");
    scanf(" %c", &grade);

    printf("\n--- User Details ---\n");
    printf("Name   : %s\n", name);
    printf("Age    : %d\n", age);
    printf("Height : %.2f\n", height);
    printf("Grade  : %c\n", grade);

    return 0;
}
