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
