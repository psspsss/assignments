
// Purpose: Create 3 threads and print their thread IDs
// Date: 08/01/2026
// Author: Vinayak Majhi
// ID: 58618
// Batch ID: 25SUB4505 - DATACOM+5G

#include <iostream>
#include <thread>
using namespace std;

void printID() { cout << "Thread ID: " << this_thread::get_id() << endl; }

int main() {
  thread t1(printID);
  thread t2(printID);
  thread t3(printID);

  t1.join();
  t2.join();
  t3.join();

  return 0;
}
