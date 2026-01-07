// Purpose: Generic Dynamic Array using Templates
// Date: 07/01/2026
// Author: Vinayak Majhi
// ID: 58618
// Batch ID: 25SUB4505 - DATACOM+5G

#include <iostream>

using namespace std;

template <class T> class DynArray {
  T *arr;
  int size;

public:
  DynArray() {
    size = 0;
    arr = nullptr;
  }

  DynArray(int s) {
    size = s;
    arr = new T[size];
  }

  DynArray(int s, T value) {
    size = s;
    arr = new T[size];
    for (int i = 0; i < size; i++)
      arr[i] = value;
  }

  void display() const {
    for (int i = 0; i < size; i++)
      cout << arr[i] << " ";
    cout << endl;
  }

  ~DynArray() { delete[] arr; }
};

int main() {
  DynArray<int> a1(5, 10);
  DynArray<float> a2(3, 2.5f);

  a1.display();
  a2.display();

  return 0;
}
