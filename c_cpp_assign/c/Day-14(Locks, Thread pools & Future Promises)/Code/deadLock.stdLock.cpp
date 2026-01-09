// Purpose: Deadlock Fix using std::lock
// Date: 09/01/2026
// Author: Vinayak Majhi
// ID: 58618
// Batch ID: 25SUB4505 - DATACOM+5G

#include <iostream>
#include <mutex>
#include <thread>

using namespace std;

mutex m1, m2;

void safeFunction() {
  lock(m1, m2); // deadlock-free locking
  lock_guard<mutex> lock1(m1, adopt_lock);
  lock_guard<mutex> lock2(m2, adopt_lock);

  cout << "Thread safely locked both mutexes\n";
}

int main() {
  thread t1(safeFunction);
  thread t2(safeFunction);

  t1.join();
  t2.join();
  return 0;
}
