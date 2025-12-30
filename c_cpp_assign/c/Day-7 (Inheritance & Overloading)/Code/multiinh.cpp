#include <iostream>
using namespace std;

class BaseOne {
public:
  void funOne() { cout << "Base::funOne()" << endl; }
  void funTwo() { cout << "Base::funTwo()" << endl; }
};

class BaseTwo {
public:
  void funOne() { cout << "Base::funOne()" << endl; }
  void funTwo() { cout << "Base::funTwo()" << endl; }
};

class Derived : public BaseOne, public BaseTwo {
public:
  using BaseOne::funOne;
  using BaseTwo::funTwo;
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
