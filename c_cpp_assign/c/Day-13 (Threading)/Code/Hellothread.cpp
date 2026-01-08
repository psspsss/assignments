// Purpose: Create a single thread and print messages from thread and main
// Date: 08/01/2026
// Author: Vinayak Majhi
// ID: 58618
// Batch ID: 25SUB4505 - DATACOM+5G

#include <iostream>
#include <thread>
using namespace std;

void threadFunc() { cout << "Hello world from thread" << endl; }

int main() {
  thread t(threadFunc);
  t.join();
  cout << "Hello world from main" << endl;
  return 0;
}
