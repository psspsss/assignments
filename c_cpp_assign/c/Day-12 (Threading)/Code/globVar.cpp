#include <iostream>
#include <mutex>
#include <thread>
using namespace std;

int globVar = 0;
mutex mVar;

void Update() {
  lock_guard<mutex> lock(mVar);
  for (int cnt = 0; cnt < 100000; cnt++)
    globVar++;
}

int main() {
  thread t1(Update);
  thread t2(Update);
  thread t3(Update);

  t1.join();
  t2.join();
  t3.join();

  cout << "global var: " << globVar << endl;
}
