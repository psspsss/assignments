// Purpose: BankAccount class without constructor using setter to initialize and manage balance
// Date: 26/12/2025
// Author: Vinayak Majhi
// ID: 58618
// Batch ID: 25SUB4505 - DATACOM+5G

#include <iostream>
#include <string>
using namespace std;

class BankAccount {
  int accNo;
  string name;
  double balance;

public:
  void setAccount(int no, string n, double bal) {
    accNo = no;
    name = n;
    balance = bal;
  }

  void deposit(double amt) { balance += amt; }

  void withdraw(double amt) {
    if (amt <= balance)
      balance -= amt;
    else
      cout << "Insufficient Balance!" << endl;
  }

  void display() {
    cout << "Account Number: " << accNo << endl;
    cout << "Account Holder: " << name << endl;
    cout << "Balance: " << balance << endl;
  }
};

int main() {
  BankAccount b1;

  b1.setAccount(101, "Rahul", 5000);
  b1.display();

  b1.deposit(2000);
  cout << "\nAfter Deposit:\n";
  b1.display();

  b1.withdraw(3000);
  cout << "\nAfter Withdrawal:\n";
  b1.display();

  return 0;
}
