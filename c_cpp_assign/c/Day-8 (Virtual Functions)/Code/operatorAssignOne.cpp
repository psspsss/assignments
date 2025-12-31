
#include <iostream>

using namespace std;

class Test {

  int data;

public:
  Test(int x = 0) : data(x) {}

  Test &operator=(int);

  Test &operator=(const Test &);

  friend ostream &operator<<(ostream &, const Test &);
};

int main() {

  Test obj;

  obj = 100;

  Test objOne;

  objOne = obj;

  cout << "Obj: " << obj << "\t\tObjOne: " << objOne << endl;
}

Test &Test::operator=(int arg) {

  cout << "Test& operator=(int arg)" << endl;

  data = arg;

  return *this;
}

Test &Test::operator=(const Test &rhs) {

  cout << "Test& Test::operator=(const Test& rhs)" << endl;

  data = rhs.data;

  return *this;
}

ostream &operator<<(ostream &out, const Test &arg) {

  out << "data: " << arg.data;

  return out;
}
