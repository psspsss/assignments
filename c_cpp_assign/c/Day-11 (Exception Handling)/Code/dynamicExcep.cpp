// Purpose: Dynamic Array class with exception handling
// Date: 06/01/2026
// Author: Vinayak Majhi
// ID: 58618
// Batch ID: 25SUB4505 - DATACOM+5G

#include <exception>
#include <iostream>

using namespace std;

class DynArray {
  int *arr;
  int size;

public:
  DynArray(int s) {
    if (s <= 0)
      throw invalid_argument("Invalid array size");

    size = s;
    arr = new int[size];
  }

  void input() {
    cout << "Enter " << size << " elements:\n";
    for (int i = 0; i < size; i++)
      cin >> arr[i];
  }

  void display() const {
    cout << "Array elements: ";
    for (int i = 0; i < size; i++)
      cout << arr[i] << " ";
    cout << endl;
  }

  ~DynArray() { delete[] arr; }
};

int main() {
  try {
    int n;
    cout << "Enter array size: ";
    cin >> n;

    DynArray d(n);
    d.input();
    d.display();
  } catch (exception &e) {
    cout << "Exception caught: " << e.what() << endl;
  }

  return 0;
}
