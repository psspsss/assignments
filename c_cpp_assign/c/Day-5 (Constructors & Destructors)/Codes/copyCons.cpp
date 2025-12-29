#include <iostream>

using namespace std;

class Test {

  int data;

public:
  Test()
      : data(0) // initialization

  {

    cout << "Test() ---> Constructor" << endl;
  }

  Test(int num)

  {

    data = num; // assignment

    cout << "Test(" << num << ") ---> Constructor" << endl;
  }

  Test(const Test &obj)

  {

    data = obj.data; // assignment

    cout << "Test(Test &) --->Copy Constructor" << endl;
  }

  ~Test() { cout << "~Test() ----> Destructor" << endl; }
};

int main() {

  Test objOne; // default ctor

  Test objTwo = 10; // single parameterized ctor

  Test objThree = objTwo; // copy ctor

  cout << "In main()" << endl;
}
