#include <iostream>
using namespace std;

struct A {
  A() { cout << "A()" << endl; }
  ~A() { cout << "~A()" << endl; }
};

struct B {
  B() { cout << "B()" << endl; }
  ~B() { cout << "~B()" << endl; }
};

struct C {
  C() { cout << "C()" << endl; }
  ~C() { cout << "~C()" << endl; }
};

struct D : A, B, C {
  D() : B(), C(), A() { cout << "D()" << endl; }
  ~D() { cout << "~D()" << endl; }
};

int main() { D dObj; }
