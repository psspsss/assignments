#include <iostream>
using namespace std;

class Test {
public:
  Test() { cout << "Test()--> Default Ctor" << endl; }
  Test(int) { cout << "Test(int)--> Parameterized Ctor" << endl; }

  ~Test() { cout << "~Test()--> Destructor" << endl; }
  Test(const Test &) { cout << "Test(const Test&) ---> Copy Ctor" << endl; }
};

Test fun(Test arg) { // call by value

  return arg;
}

int main() {
  Test objTwo = 100; // single parameterized ctor

  fun(objTwo);
}
