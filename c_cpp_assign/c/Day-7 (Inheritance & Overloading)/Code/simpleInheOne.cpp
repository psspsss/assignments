#include <iostream>
using namespace std;

class Base {
public:
  void funOne() { cout << "Base::funOne()" << endl; }
  void funTwo() { cout << "Base::funTwo()" << endl; }
};

class Derived : public Base {
public:
  void funThree() { cout << "Derived::funThree" << endl; }
  void funFour() { cout << "Derived::funFour" << endl; }
};

int main() {
  Derived dObj;
  dObj.funOne();
  dObj.funTwo();
  dObj.funThree();
  dObj.funFour();
}
