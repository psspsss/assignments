#include <iostream>
using namespace std;

class MyArray {
  int arr[100];
  int size;

public:
  MyArray() { size = 0; }

  MyArray(int data) {
    arr[0] = data;
    size = 1;
  }

  MyArray(const MyArray &a) {
    size = a.size;
    for (int i = 0; i < size; i++)
      arr[i] = a.arr[i];
  }

  void printArray() {
    cout << "Array: ";
    for (int i = 0; i < size; i++)
      cout << arr[i] << " ";
    cout << endl;
  }

  void extend(const MyArray &a) {
    for (int i = 0; i < a.size; i++) {
      arr[size] = a.arr[i];
      size++;
    }
  }
};

int main() {
  MyArray a1(10);
  MyArray a2(20);

  cout << "Array 1: ";
  a1.printArray();

  cout << "Array 2: ";
  a2.printArray();

  MyArray a3(a1);
  a3.extend(a2);

  cout << "Extended Array: ";
  a3.printArray();

  return 0;
}
