// Purpose: Demonstrate deadlock scenario
// Date: 09/01/2026
// Author: Vinayak Majhi
// ID: 58618
// Batch ID: 25SUB4505 - DATACOM+5G

#include <iostream>
#include <mutex>
#include <thread>
using namespace std;

mutex m1, m2;

void threadA() {
  lock_guard<mutex> lg1(m1);
  this_thread::sleep_for(chrono::milliseconds(100));
  lock_guard<mutex> lg2(m2);
}

void threadB() {
  lock_guard<mutex> lg1(m2);
  this_thread::sleep_for(chrono::milliseconds(100));
  lock_guard<mutex> lg2(m1);
}

int main() {
  thread t1(threadA);
  thread t2(threadB);

  t1.join();
  t2.join();
}
