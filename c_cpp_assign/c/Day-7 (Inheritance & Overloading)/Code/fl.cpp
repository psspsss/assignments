#include <iostream>
using namespace std;

class Num {
  int data;

public:
  Num(int x = 0) : data(x) {}
  friend Num operator+(const Num &, const Num &);
  friend Num operator-(const Num &, const Num &);
  friend ostream &operator<<(ostream &, const Num &);
};

Num operator+(const Num &lhs, const Num &rhs) {
  Num temp(lhs.data + rhs.data);
  return temp;
}

Num operator-(const Num &lhs, const Num &rhs) {
  Num temp(lhs.data - rhs.data);
  return temp;
}

ostream &operator<<(ostream &out, const Num &obj) {
  out << "data: " << obj.data;
  return out;
}

int main() {
  Num a = 100, b = 20;
  cout << a << "   " << b << endl;
  Num c = a + b;
  Num d = c + a - b;
  cout << c << "   " << d << endl;
}
