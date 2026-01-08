// Purpose: Fill a global array of 100 elements using 5 threads (20 elements per
// thread) Date: 08/01/2026 Author: Vinayak Majhi ID: 58618 Batch ID: 25SUB4505
// - DATACOM+5G

#include <iostream>
#include <thread>
using namespace std;

int arr[100];

void fillArray(int startIndex, int count) {
  for (int i = startIndex; i < startIndex + count; i++) {
    arr[i] = i + 1;
  }
}

int main() {
  thread t1(fillArray, 0, 20);  // Thread 0 → index 0–19
  thread t2(fillArray, 20, 20); // Thread 1 → index 20–39
  thread t3(fillArray, 40, 20); // Thread 2 → index 40–59
  thread t4(fillArray, 60, 20); // Thread 3 → index 60–79
  thread t5(fillArray, 80, 20); // Thread 4 → index 80–99

  t1.join();
  t2.join();
  t3.join();
  t4.join();
  t5.join();

  for (int i = 0; i < 100; i++) {
    cout << arr[i] << " ";
  }
  cout << endl;

  return 0;
}
