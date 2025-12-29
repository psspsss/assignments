// Purpose: Demonstrate copy constructor call count and shallow vs deep copy
// Date: 29/12/2025
// Author: Vinayak Majhi
// ID: 58618
// Batch ID: 25SUB4505 - DATACOM+5G

#include <cstring>
#include <iostream>

using namespace std;

class Sample {
  int *ptr;
  static int copyCount;

public:
  Sample(int val = 0) { ptr = new int(val); }

  Sample(const Sample &obj) {
    copyCount++;
    ptr = new int(*obj.ptr);
  }

  static int getCopyCount() { return copyCount; }

  ~Sample() { delete ptr; }
};

int Sample::copyCount = 0;

void testFunction(Sample s) {}

int main() {

  Sample a(10);
  testFunction(a);
  testFunction(a);

  cout << "Copy Constructor called: " << Sample::getCopyCount() << " times"
       << endl;

  return 0;
}
