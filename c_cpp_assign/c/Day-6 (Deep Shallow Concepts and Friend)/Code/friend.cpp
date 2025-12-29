#include <iostream>
using namespace std;

class Test { // all members of a class is private by default
  int data;
  Test(int x = 10) : data(x) { cout << "constructor called" << endl; }
  void disp() { cout << "Data: " << data << endl; }

  friend int main();
};

int main() {
  Test obj = 100;
  obj.disp();
}
