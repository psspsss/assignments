// Purpose: Insert a value at a specified index in a dynamic array and display the result
// Date: 24/12/2025
// Author: Vinayak Majhi
// ID: 58618
// Batch ID: 25SUB4505 - DATACOM+5G

#include <cstdlib>
#include <ctime>
#include <iostream>

using namespace std;

int main() {
  int n, pos, value;

  cout << "Enter size: ";
  cin >> n;

  int *arr = new int[n];
  srand(time(0));

  for (int i = 0; i < n; i++) {
    arr[i] = rand() % 100;
    cout << arr[i] << " ";
  }

  cout << "\nEnter position to insert (0 to " << n << "): ";
  cin >> pos;
  cout << "Enter value: ";
  cin >> value;

  int *newArr = new int[n + 1];

  for (int i = 0; i < pos; i++) {
    newArr[i] = arr[i];
  }

  newArr[pos] = value;

  for (int i = pos; i < n; i++) {
    newArr[i + 1] = arr[i];
  }

  cout << "\nAfter insertion:\n";
  for (int i = 0; i < n + 1; i++) {
    cout << newArr[i] << " ";
  }

  delete[] arr;
  delete[] newArr;

  return 0;
}
