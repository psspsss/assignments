// Purpose: Classify educational stage by age and read and print a user's name
// Date: 23/12/2025
// Author: Vinayak Majhi
// ID: 58618
// Batch ID: 25SUB4505 - DATACOM+5G

#include <stdio.h>

int main() {

  int age = 0;
  printf("Enter age: ");
  scanf("%d", &age);

  if (age <= 4) {
    printf("Not in school");
  }

  else if (age >= 5 && age < 11) {
    printf("Elementary School");
  }

  else if (age >= 11 && age < 14) {
    printf("Middle School");

  } else if (age >= 14 && age < 16) {
    printf("High School");

  } else if (age >= 16 && age < 18) {
    printf("Pre-University");
  }

  else if (age >= 18 && age < 21) {
    printf("Under Graduation");
  }

  else {
    printf("Adult");
  }

  char name[25] = {};
  printf("\nEnter Name: ");
  scanf(" %[^\n]", name);
  printf("%s", name);

  return 0;
}
