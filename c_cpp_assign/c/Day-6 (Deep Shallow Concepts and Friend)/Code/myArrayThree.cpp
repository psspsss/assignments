#include <iostream>

using namespace std;

class MyArray { // abstraction

  int arr[100];

  int size;

public:
  MyArray();

  MyArray(int data);

  void printArray();
};

int main() {

  MyArray objTwo(100); // single parameterized ctor

  MyArray objThree = objTwo; // copying one object to another

  objTwo.printArray();

  objThree.printArray();
}

MyArray::MyArray() {

  size = 100;

  for (int cnt = 0; cnt < size; cnt++)

    arr[cnt] = 0;
}

MyArray::MyArray(int data) {

  size = 100;

  for (int cnt = 0; cnt < size; cnt++)

    arr[cnt] = data + cnt;
}

void MyArray::printArray() {

  cout << "Size: " << size << "\tarray: " << endl;

  for (int cnt = 0; cnt < size; cnt++)

    cout << arr[cnt] << "  ";

  cout << endl;
}
