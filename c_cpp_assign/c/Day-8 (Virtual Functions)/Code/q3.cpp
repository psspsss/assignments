// Purpose: Banking system using virtual functions and base references
// Date: 31/12/2025
// Author: Vinayak Majhi
// ID: 58618
// Batch ID: 25SUB4505 - DATACOM+5G

#include <iostream>
using namespace std;

class Account {
protected:
  double balance;

public:
  Account(double b) : balance(b) {}

  virtual void calculateInterest() = 0;
  virtual void display() = 0;

  virtual ~Account() {}
};

class SavingsAccount : public Account {
public:
  SavingsAccount(double b) : Account(b) {}

  void calculateInterest() override { balance += balance * 0.04; }

  void display() override {
    cout << "Savings Account Balance: " << balance << endl;
  }
};

class CurrentAccount : public Account {
public:
  CurrentAccount(double b) : Account(b) {}

  void calculateInterest() override {}

  void display() override {
    cout << "Current Account Balance: " << balance << endl;
  }
};

class FixedDepositAccount : public Account {
public:
  FixedDepositAccount(double b) : Account(b) {}

  void calculateInterest() override { balance += balance * 0.07; }

  void display() override {
    cout << "Fixed Deposit Balance: " << balance << endl;
  }
};

int main() {

  SavingsAccount sa(10000);
  FixedDepositAccount fd(20000);

  Account &a1 = sa;
  Account &a2 = fd;

  a1.calculateInterest();
  a2.calculateInterest();

  a1.display();
  a2.display();

  return 0;
}
