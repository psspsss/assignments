
// Purpose: Print numbers from 1 to 10 using lambda in thread
// Date: 08/01/2026
// Author: Vinayak Majhi
// ID: 58618
// Batch ID: 25SUB4505 - DATACOM+5G

#include <iostream>
#include <thread>
using namespace std;

int main() {
  thread t([]() {
    for (int i = 1; i <= 10; i++)
      cout << i << " ";
    cout << endl;
  });

  t.join();
  return 0;
}
