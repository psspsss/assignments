// Purpose: Demonstrate double deletion using shallow copy
// Date: 29/12/2025
// Author: Vinayak Majhi
// ID: 58618
// Batch ID: 25SUB4505 - DATACOM+5G

#include <cstring>
#include <iostream>

using namespace std;

class Resource {
public:
  char *ptr;

  Resource(const char *name) {
    ptr = new char[strlen(name) + 1];
    strcpy(ptr, name);
  }

  ~Resource() { delete[] ptr; }
};

int main() {

  Resource r1("Memory");
  Resource r2 = r1;

  return 0;
}
