// Purpose: Populate dynamic array with random numbers and compute sum and average
// Date: 24/12/2025
// Author: Vinayak Majhi
// ID: 58618
// Batch ID: 25SUB4505 - DATACOM+5G

#include <cstdlib>
#include <ctime>
#include <iostream>

using namespace std;

int main() {
  int n, sum = 0;
  cout << "Enter size: ";
  cin >> n;

  int *arr = new int[n];
  srand(time(0));

  for (int i = 0; i < n; i++) {
    arr[i] = rand() % 100;
    sum += arr[i];
    cout << arr[i] << " ";
  }

  float avg = (float)sum / n;

  cout << "\nSum = " << sum;
  cout << "\nAverage = " << avg;

  delete[] arr;
  return 0;
}
