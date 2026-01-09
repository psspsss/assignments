#include <chrono>
#include <condition_variable>
#include <functional>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>
using namespace std;

class SimpleThreadPool {
  vector<thread> workers; // stores thread objects
  queue<function<void()>> tasks;

  mutex mLock;
  condition_variable cLock;
  bool stop = false;

public:
  SimpleThreadPool(unsigned int num) {
    for (unsigned int i = 0; i < num; ++i) {
      workers.emplace_back([this] {
        while (true) {
          function<void()> task;
          {
            unique_lock<mutex> lock(mLock);
            cLock.wait(lock, [this] { return stop | !tasks.empty(); });

            if (stop && tasks.empty())
              return;

            task = move(tasks.front());
            tasks.pop();
          }

          task();
        }
      });
    }
  }

  void submit(function<void()> task) {
    {
      lock_guard<mutex> lock(mLock);
      tasks.push(task);
    }
    cLock.notify_one();
  }

  ~SimpleThreadPool() {
    {
      lock_guard<mutex> lock(mLock);
      stop = true;
    }
    cLock.notify_all();
    for (auto &t : workers)
      t.join();
  }
};

int main() {
  SimpleThreadPool pool(3);

  for (int i = 0; i < 6; ++i) {
    pool.submit([i] {
      cout << "Task " << i + 1 << " executed by thread "
           << this_thread::get_id() << "\n";
      this_thread::sleep_for(chrono::seconds(1));
    });
  }
}
