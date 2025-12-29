// Purpose: Find minimum and maximum values in an array using reference parameters
// Date: 26/12/2025
// Author: Vinayak Majhi
// ID: 58618
// Batch ID: 25SUB4505 - DATACOM+5G

#include <iostream>
using namespace std;

void findMinMax(int *arr, int size, int &min, int &max) {
  min = max = arr[0];

  for (int i = 1; i < size; i++) {
    if (arr[i] < min)
      min = arr[i];
    if (arr[i] > max)
      max = arr[i];
  }
}

int main() {
  int arr[] = {12, 5, 30, 7, 18};
  int min, max;

  findMinMax(arr, 5, min, max);

  cout << "Minimum value = " << min << endl;
  cout << "Maximum value = " << max << endl;

  return 0;
}
