#include <iostream>
using namespace std;

class Base {
public:
  virtual void disp() = 0; // definition is missing
};
class Derived : public Base {
public:
  void disp() { cout << "Derived::disp()" << endl; }
};

int main() {
  Base *bPtr; // object of Base class cannot be created
  Derived dObj;

  bPtr = &dObj; // storing derived class object
  bPtr->disp(); //--> (2) calls derived class function
}
