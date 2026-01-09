// Purpose: Demonstrate Deadlock Scenario
// Date: 09/01/2026
// Author: Vinayak Majhi
// ID: 58618
// Batch ID: 25SUB4505 - DATACOM+5G

#include <chrono>
#include <iostream>
#include <mutex>
#include <thread>

using namespace std;

mutex m1, m2;

void threadA() {
  m1.lock();
  cout << "Thread A locked m1\n";
  this_thread::sleep_for(chrono::milliseconds(100));

  m2.lock(); // waits forever
  cout << "Thread A locked m2\n";

  m2.unlock();
  m1.unlock();
}

void threadB() {
  m2.lock();
  cout << "Thread B locked m2\n";
  this_thread::sleep_for(chrono::milliseconds(100));

  m1.lock(); // waits forever
  cout << "Thread B locked m1\n";

  m1.unlock();
  m2.unlock();
}

int main() {
  thread t1(threadA);
  thread t2(threadB);

  t1.join();
  t2.join();

  return 0;
}
