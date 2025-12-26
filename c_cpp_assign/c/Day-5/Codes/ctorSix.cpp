#include <cstring>
#include <iostream>
#include <string>
using namespace std;

class Test {
  string name; // c++ style string
  int num;

public:
  Test(const char arg[] = "Some string") : name(arg), num(strlen(arg)) {}
  void disp() { cout << "Num: " << num << "\tname: " << name << endl; }
  ~Test() {}
};

int main() {
  Test obj;
  obj.disp();
}
