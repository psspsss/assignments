// Purpose: Demonstrate mutex synchronization using unique_lock (manual unlock &
// deferred lock) Date: 08/01/2026 Author: Vinayak Majhi ID: 58618 Batch ID:
// 25SUB4505 - DATACOM+5G

#include <iostream>
#include <mutex>
#include <thread>
using namespace std;

mutex mVar;
int counter = 0;

/* ---------- Manual lock / unlock using unique_lock ---------- */
void incrementManual() {
  unique_lock<mutex> lock(mVar);
  counter++;
  cout << "Manual Lock Counter: " << counter << endl;
  lock.unlock();
}

/* ---------- Deferred lock using unique_lock ---------- */
void incrementDeferred() {
  unique_lock<mutex> lock(mVar, defer_lock);
  lock.lock();
  counter++;
  cout << "Deferred Lock Counter: " << counter << endl;
}

int main() {
  thread t1(incrementManual);
  thread t2(incrementManual);

  t1.join();
  t2.join();

  thread t3(incrementDeferred);
  thread t4(incrementDeferred);

  t3.join();
  t4.join();

  return 0;
}
