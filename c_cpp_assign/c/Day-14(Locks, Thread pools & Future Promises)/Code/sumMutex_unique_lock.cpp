// Purpose: Sum numbers from 1 to 1000 using multiple threads (unique_lock)
// Date: 09/01/2026
// Author: Vinayak Majhi
// ID: 58618
// Batch ID: 25SUB4505 - DATACOM+5G

#include <iostream>
#include <mutex>
#include <thread>
using namespace std;

long long sum = 0;
mutex m;

void addRange(int start, int end) {
  for (int i = start; i <= end; i++) {
    unique_lock<mutex> ul(m);
    sum += i;
  }
}

int main() {
  thread t1(addRange, 1, 333);
  thread t2(addRange, 334, 666);
  thread t3(addRange, 667, 1000);

  t1.join();
  t2.join();
  t3.join();

  cout << "Final Sum = " << sum << endl;
  return 0;
}
