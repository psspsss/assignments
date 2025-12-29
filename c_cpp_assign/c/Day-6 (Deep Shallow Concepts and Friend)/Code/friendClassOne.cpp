#include <iostream>

using namespace std;

class UsingTest;

class Test { // all members of a class is private by default

  int data;

  Test(int x = 10) : data(x) { cout << "constructor called" << endl; }

  void disp() { cout << "Data: " << data << endl; }

  friend class UsingTest;
};

class UsingTest {

  Test obj;

public:
  UsingTest(int x = 100) : obj(Test(x)) {}

  void print() { obj.disp(); }
};

int main() {

  UsingTest obj = 100;

  obj.print();
}
