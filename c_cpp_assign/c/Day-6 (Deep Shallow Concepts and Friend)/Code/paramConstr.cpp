#include <iostream>
using namespace std;

class MyArray { // abstraction
  int *arr;     // array here is a pointer variable
  int size;     // integer
public:
  MyArray();
  MyArray(int data);
  void printArray();

  ~MyArray() {
    if (size) {
      // delete [] arr;
    }
    size = 0;
  }
};

int main() {
  MyArray objTwo(100);       // single parameterized ctor
  MyArray objThree = objTwo; // copying one object to another

  objTwo.printArray();
  objThree.printArray();
}

MyArray::MyArray() : size(10) {
  arr = new int[size];
  for (int cnt = 0; cnt < size; cnt++)
    arr[cnt] = 0;
}

MyArray::MyArray(int sz) : size(sz) {
  int data = 101;
  arr = new int[size];
  for (int cnt = 0; cnt < size; cnt++)
    arr[cnt] = data + cnt;
}

void MyArray::printArray() {
  cout << "Size: " << size << "\tarray: " << endl;
  for (int cnt = 0; cnt < size; cnt++)
    cout << arr[cnt] << "  ";
  cout << endl;
}
