// Purpose: Demonstrate pointer variables and print addresses for different primitive types
// Date: 24/12/2025
// Author: Vinayak Majhi
// ID: 58618
// Batch ID: 25SUB4505 - DATACOM+5G

#include <iostream>
using namespace std;

int main() {
  int a = 10;
  float b = 3.14f;
  double c = 9.81;

  int *iPtr = &a;
  float *fPtr = &b;
  double *dPtr = &c;

  // iPtr = dPtr;

  cout << "Address stored in iPtr: " << iPtr << endl;
  cout << "Address stored in fPtr: " << fPtr << endl;
  cout << "Address stored in dPtr: " << dPtr << endl;

  return 0;
}
