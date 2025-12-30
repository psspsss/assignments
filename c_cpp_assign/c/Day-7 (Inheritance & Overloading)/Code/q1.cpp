// Purpose: Demonstrate operator overloading using Complex number class
// Date: 30/12/2025
// Author: Vinayak Majhi
// ID: 58618
// Batch ID: 25SUB4505 - DATACOM+5G

#include <iostream>

using namespace std;

class Complex {
  int real;
  int imag;

public:
  Complex(int r = 0, int i = 0) {
    real = r;
    imag = i;
  }

  Complex operator+(const Complex &c) {
    return Complex(real + c.real, imag + c.imag);
  }

  Complex operator-(const Complex &c) {
    return Complex(real - c.real, imag - c.imag);
  }

  friend ostream &operator<<(ostream &out, const Complex &c);
};

ostream &operator<<(ostream &out, const Complex &c) {
  out << c.real;
  if (c.imag >= 0)
    out << " + " << c.imag << "i";
  else
    out << " - " << -c.imag << "i";
  return out;
}

int main() {

  Complex c1(3, 4), c2(1, 2);

  Complex c3 = c1 + c2;
  Complex c4 = c1 - c2;

  cout << "c1 = " << c1 << endl;
  cout << "c2 = " << c2 << endl;
  cout << "c1 + c2 = " << c3 << endl;
  cout << "c1 - c2 = " << c4 << endl;

  return 0;
}
