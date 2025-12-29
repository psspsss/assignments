// Purpose: Fill an array with random numbers using srand/rand and print them
// Date: 24/12/2025
// Author: Vinayak Majhi
// ID: 58618
// Batch ID: 25SUB4505 - DATACOM+5G

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main() {

  srand(time(NULL)); // Seed for random function

  int arr[10], cnt;
  for (cnt = 0; cnt < 10; cnt++)
    arr[cnt] = rand() % 100;

  printf("Arr: ");
  for (cnt = 0; cnt < 10; cnt++)
    printf("%d  ", arr[cnt]);
  printf("\n");
}
