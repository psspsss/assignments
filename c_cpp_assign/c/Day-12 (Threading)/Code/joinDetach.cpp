
// Purpose: Demonstrate join and detach behavior in threads
// Date: 08/01/2026
// Author: Vinayak Majhi
// ID: 58618
// Batch ID: 25SUB4505 - DATACOM+5G

#include <chrono>
#include <iostream>
#include <thread>
using namespace std;

void printMessage() { cout << "Hello world from thread" << endl; }

int main() {
  thread t1(printMessage);
  thread t2(printMessage);

  t1.join();
  t2.detach();

  cout << "Hello world from main" << endl;

  this_thread::sleep_for(chrono::seconds(1));
  return 0;
}
