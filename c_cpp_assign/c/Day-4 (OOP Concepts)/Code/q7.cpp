// Purpose: Allocate, populate, and print a dynamic 2D array using pointer-to-pointer
// Date: 24/12/2025
// Author: Vinayak Majhi
// ID: 58618
// Batch ID: 25SUB4505 - DATACOM+5G

#include <iostream>
using namespace std;

int main() {
  int rows, cols;

  cout << "Enter rows: ";
  cin >> rows;
  cout << "Enter columns: ";
  cin >> cols;

  int **arr = new int *[rows];
  for (int i = 0; i < rows; i++) {
    arr[i] = new int[cols];
  }

  cout << "Enter elements:\n";
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < cols; j++) {
      cin >> arr[i][j];
    }
  }

  cout << "\n2D Array:\n";
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < cols; j++) {
      cout << arr[i][j] << " ";
    }
    cout << endl;
  }

  for (int i = 0; i < rows; i++) {
    delete[] arr[i];
  }
  delete[] arr;

  return 0;
}
