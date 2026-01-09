#include <chrono>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <thread>
using namespace std;

mutex mVar;
condition_variable cVar;
bool ready = false;
void funOne() {
  unique_lock<mutex> lock(mVar);
  cVar.wait(lock, [] { return ready; });
  cout << "Recieved Notification.. Proceeding now " << ready << endl;
}
int main() {
  thread t1(funOne);
  this_thread::sleep_for(chrono::seconds(2));
  {
    lock_guard<mutex> lock(mVar);
    ready = true;
  }
  cVar.notify_one();
  t1.join();
}
