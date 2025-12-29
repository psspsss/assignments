// Purpose: Initialize std::string via constructor and compute length using initializer list
// Date: 26/12/2025
// Author: Vinayak Majhi
// ID: 58618
// Batch ID: 25SUB4505 - DATACOM+5G

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
