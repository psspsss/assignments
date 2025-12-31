#include <iostream>

using namespace std;

class Base {

public:
  virtual void funOne() { cout << "Base::funOne()" << endl; }

  virtual void funTwo() { cout << "Base::funTwo()" << endl; }

  virtual void funThree() { cout << "Base::funThree()" << endl; }
};

class Derived : public Base {

public:
  void funOne() { cout << "Derived::funOne()" << endl; }

  void funTwo() { cout << "Derived::funTwo()" << endl; }

  void funThree() { cout << "Derived::funThree()" << endl; }
};

void demoVirtFun(Base *bPtr) { // polymorphism using Base class Pointer

  cout << "Using Base class Pointer variable" << endl;

  bPtr->funOne();

  bPtr->funTwo();

  bPtr->funThree();

  cout << "------------------------------------------\n";
}

void demoVirtFun(
    Base &bPtr) { // polymorphism using Base class reference variable

  cout << "Using Base class Reference variable" << endl;

  bPtr.funOne();

  bPtr.funTwo();

  bPtr.funThree();

  cout << "------------------------------------------\n";
}

int main() {

  Base bObj;

  Derived dObj;

  demoVirtFun(&bObj);

  demoVirtFun(&dObj);
}
