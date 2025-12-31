#include <iostream>
using namespace std;

class Base {
public:
  virtual void disp() { cout << "Base::disp()" << endl; }
};
class Derived : public Base {
public:
  void disp() { cout << "Derived::disp()" << endl; }
};

int main() {
  Base *bPtr, bObj;
  Derived dObj;

  bPtr = &bObj; // storing base class object
  bPtr->disp(); //--> (1) calls base class function

  bPtr = &dObj; // storing derived class object
  bPtr->disp(); //--> (2) calls derived class function
}
