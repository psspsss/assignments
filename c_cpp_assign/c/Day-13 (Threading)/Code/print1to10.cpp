
// Purpose: Print numbers from 1 to 10 using function argument in thread
// Date: 08/01/2026
// Author: Vinayak Majhi
// ID: 58618
// Batch ID: 25SUB4505 - DATACOM+5G

#include <iostream>
#include <thread>
using namespace std;

void printNumbers() {
  for (int i = 1; i <= 10; i++)
    cout << i << " ";
  cout << endl;
}

int main() {
  thread t(printNumbers);
  t.join();
  return 0;
}
