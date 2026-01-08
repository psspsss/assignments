// Purpose: Create multiple threads using vector and synchronize with join
// Date: 08/01/2026
// Author: Vinayak Majhi
// ID: 58618
// Batch ID: 25SUB4505 - DATACOM+5G

#include <iostream>
#include <thread>
#include <vector>
using namespace std;

void task(int id) { cout << "Hello world from thread " << id << endl; }

int main() {
  vector<thread> threads;

  for (int i = 1; i <= 5; i++)
    threads.emplace_back(task, i);

  for (auto &t : threads)
    t.join();

  cout << "All threads finished execution" << endl;
  return 0;
}
