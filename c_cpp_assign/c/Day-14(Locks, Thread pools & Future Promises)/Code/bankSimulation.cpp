// Purpose: Bank account simulation with thread-safe methods
// Date: 09/01/2026
// Author: Vinayak Majhi
// ID: 58618
// Batch ID: 25SUB4505 - DATACOM+5G

#include <iostream>
#include <mutex>
#include <thread>
using namespace std;

class BankAccount {
  int balance;
  mutex m;

public:
  BankAccount(int bal) : balance(bal) {}

  void deposit(int amt) {
    lock_guard<mutex> lg(m);
    balance += amt;
  }

  void withdraw(int amt) {
    lock_guard<mutex> lg(m);
    if (balance >= amt)
      balance -= amt;
  }

  int getBalance() { return balance; }
};

int main() {
  BankAccount acc(1000);

  thread t1(&BankAccount::deposit, &acc, 500);
  thread t2(&BankAccount::withdraw, &acc, 300);
  thread t3(&BankAccount::deposit, &acc, 200);

  t1.join();
  t2.join();
  t3.join();

  cout << "Final Balance = " << acc.getBalance() << endl;
  return 0;
}
