// Purpose: Print 1–10 using 3 threads with IO synchronization
// Date: 09/01/2026
// Author: Vinayak Majhi
// ID: 58618
// Batch ID: 25SUB4505 - DATACOM+5G

#include <iostream>
#include <mutex>
#include <thread>
using namespace std;

mutex ioMutex;

void printSeries(int id) {
  lock_guard<mutex> lg(ioMutex);
  cout << "Thread #" << id << ": ";
  for (int i = 1; i <= 10; i++)
    cout << i << " ";
  cout << endl;
}

int main() {
  thread t1(printSeries, 1);
  thread t2(printSeries, 2);
  thread t3(printSeries, 3);

  t1.join();
  t2.join();
  t3.join();
  return 0;
}
