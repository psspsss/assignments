// Purpose: Compare swapping via pointer and swapping via reference functions with examples
// Date: 26/12/2025
// Author: Vinayak Majhi
// ID: 58618
// Batch ID: 25SUB4505 - DATACOM+5G

#include <iostream>
using namespace std;

void swap(int *a, int *b) {
  int temp = *a;
  *a = *b;
  *b = temp;
}

void swap(int &a, int &b) {
  int temp = a;
  a = b;
  b = temp;
}

int main() {
  int x = 10, y = 20;

  cout << "Before swap (Pointer): x = " << x << ", y = " << y << endl;
  swap(&x, &y);
  cout << "After swap (Pointer):  x = " << x << ", y = " << y << endl;

  cout << endl;

  cout << "Before swap (Reference): x = " << x << ", y = " << y << endl;
  swap(x, y);
  cout << "After swap (Reference):  x = " << x << ", y = " << y << endl;

  return 0;
}
