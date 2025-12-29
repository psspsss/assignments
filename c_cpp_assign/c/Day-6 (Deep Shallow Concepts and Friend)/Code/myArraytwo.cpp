#include <iostream>

using namespace std;

class MyArray { // abstraction

  static const int max = 100;

  int arr[max];

  int size;

public:
  MyArray(); // size = 10

  MyArray(int data); // size = 20

  MyArray(const MyArray &); //

  void printArray();

  MyArray extend(MyArray &); // 10 + 20 --> less than 100(max)
};

int main() {

  MyArray objOne;

  MyArray objTwo(100);

  MyArray objThree = objTwo;

  objOne.printArray();

  objTwo.printArray();

  objThree.printArray();

  MyArray objFour = objOne.extend(objThree);

  objFour.printArray();
}

MyArray::MyArray() {

  size = 10;

  for (int cnt = 0; cnt < size; cnt++)

    arr[cnt] = 0;
}

MyArray::MyArray(int data) {

  size = 20;

  for (int cnt = 0; cnt < size; cnt++)

    arr[cnt] = data + cnt;
}

MyArray::MyArray(const MyArray &rhsObj) {

  size = rhsObj.size;

  for (int cnt = 0; cnt < size; cnt++)

    arr[cnt] = rhsObj.arr[cnt];
}

void MyArray::printArray() {

  cout << "Size: " << size << "\tarray: " << endl;

  for (int cnt = 0; cnt < size; cnt++)

    cout << arr[cnt] << "  ";

  cout << endl;
}

MyArray MyArray::extend(MyArray &rhsObj) {

  // For future implementation with dynamic memory allocation

  MyArray newObj;

  newObj.size = size + rhsObj.size;

  for (int cnt = 0; cnt < size; cnt++)

    newObj.arr[cnt] = arr[cnt];

  for (int nCnt = 0, cnt = size; cnt < newObj.size; cnt++)

    newObj.arr[cnt] = rhsObj.arr[nCnt++];

  return newObj;
}
