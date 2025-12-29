// Purpose: Find and print largest and smallest elements in a dynamically allocated array
// Date: 24/12/2025
// Author: Vinayak Majhi
// ID: 58618
// Batch ID: 25SUB4505 - DATACOM+5G

#include <cstdlib>
#include <ctime>
#include <iostream>

using namespace std;

int main() {
  int n;
  cout << "Enter size: ";
  cin >> n;

  int *arr = new int[n];
  srand(time(0));

  for (int i = 0; i < n; i++) {
    arr[i] = rand() % 100;
    cout << arr[i] << " ";
  }

  int max = arr[0], min = arr[0];

  for (int i = 1; i < n; i++) {
    if (arr[i] > max)
      max = arr[i];
    if (arr[i] < min)
      min = arr[i];
  }

  cout << "\nLargest = " << max;
  cout << "\nSmallest = " << min;

  delete[] arr;
  return 0;
}
