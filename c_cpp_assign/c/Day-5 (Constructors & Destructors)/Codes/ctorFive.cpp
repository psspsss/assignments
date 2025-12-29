// Purpose: Constructor with default C-string parameter copying content and storing its length
// Date: 26/12/2025
// Author: Vinayak Majhi
// ID: 58618
// Batch ID: 25SUB4505 - DATACOM+5G

#include <cstring>
#include <iostream>
using namespace std;

class Test {
  char name[30]; // c style string //char *name;
  int len;

public:
  Test(const char arg[] = "Some string") {
    strcpy(name, arg);
    len = strlen(arg);
  }
  void disp() { cout << "Len: " << len << "\tname: " << name << endl; }
  ~Test() {}
};

int main() {
  Test obj;
  obj.disp();
}
