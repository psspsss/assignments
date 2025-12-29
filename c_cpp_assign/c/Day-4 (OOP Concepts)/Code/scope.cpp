// Purpose: Demonstrate variable scope and the scope resolution operator in C++
// Date: 24/12/2025
// Author: Vinayak Majhi
// ID: 58618
// Batch ID: 25SUB4505 - DATACOM+5G

#include <iostream>

using namespace std;

int var = 100;

int main() {

  int var = 200;
  cout << "gvar " << var << "\nlvar: " << ::var << endl;
}
