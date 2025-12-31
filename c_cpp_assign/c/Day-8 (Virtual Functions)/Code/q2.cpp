// Purpose: Demonstrate abstract class and polymorphism
// Date: 31/12/2025
// Author: Vinayak Majhi
// ID: 58618
// Batch ID: 25SUB4505 - DATACOM+5G

#include <iostream>
using namespace std;

class PaymentMethod {
public:
  virtual void pay(double amount) = 0;
  virtual ~PaymentMethod() {}
};

class CreditCard : public PaymentMethod {
public:
  void pay(double amount) override {
    cout << "Paid " << amount << " using Credit Card" << endl;
  }
};

class DebitCard : public PaymentMethod {
public:
  void pay(double amount) override {
    cout << "Paid " << amount << " using Debit Card" << endl;
  }
};

class NetBanking : public PaymentMethod {
public:
  void pay(double amount) override {
    cout << "Paid " << amount << " using Net Banking" << endl;
  }
};

class UPI : public PaymentMethod {
public:
  void pay(double amount) override {
    cout << "Paid " << amount << " using UPI" << endl;
  }
};

int main() {

  PaymentMethod *p;

  CreditCard cc;
  DebitCard dc;
  UPI u;

  p = &cc;
  p->pay(500);

  p = &dc;
  p->pay(1200);

  p = &u;
  p->pay(250);

  return 0;
}
