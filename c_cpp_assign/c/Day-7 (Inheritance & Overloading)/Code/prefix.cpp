#include <iostream>
using namespace std;

class Num {
  int data;

public:
  Num(int x = 0) : data(x) {}
  Num &operator++() {
    data++;
    return *this;
  }

  void disp(ostream &out) { out << "data: " << data << endl; }
};

int main() {
  Num a = 10;
  ++a;
  a.disp(cout);
  ++a;
  a.disp(cout);
}
