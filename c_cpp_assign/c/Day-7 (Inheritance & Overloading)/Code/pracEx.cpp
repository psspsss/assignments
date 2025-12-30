#include <iostream>
using namespace std;

class Num {
  int data;

public:
  Num(int x = 0) : data(x) {}
  Num operator+(const Num &rhs) {
    Num temp(data + rhs.data);
    return temp;
  }

  Num operator-(const Num &rhs) {
    Num temp(data - rhs.data);
    return temp;
  }
  void disp(ostream &out) { out << "data: " << data << endl; }
};

int main() {
  Num a = 100, b = 20;
  a.disp(cout);
  b.disp(cout);
  Num c = a + b;
  Num d = c + a - b;
  c.disp(cout);
  d.disp(cout);
}
