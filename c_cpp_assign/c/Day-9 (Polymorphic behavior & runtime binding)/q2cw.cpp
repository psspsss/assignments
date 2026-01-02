// Purpose: Employee Payroll System using runtime polymorphism
// Date: 02/01/2026
// Author: Vinayak Majhi
// ID: 58618
// Batch ID: 25SUB4505 - DATACOM+5G

#include <iostream>
using namespace std;

class Employee {
protected:
  int empId;
  string name;

public:
  Employee(int id, string n) : empId(id), name(n) {}

  virtual void calculateSalary() = 0;
  virtual ~Employee() {}
};

class FullTimeEmployee : public Employee {
  double basicPay, allowances;

public:
  FullTimeEmployee(int id, string n, double b, double a)
      : Employee(id, n), basicPay(b), allowances(a) {}

  void calculateSalary() override {
    cout << "Salary: " << basicPay + allowances << endl;
  }
};

class PartTimeEmployee : public Employee {
  int hoursWorked;
  double payPerHour;

public:
  PartTimeEmployee(int id, string n, int h, double p)
      : Employee(id, n), hoursWorked(h), payPerHour(p) {}

  void calculateSalary() override {
    cout << "Salary: " << hoursWorked * payPerHour << endl;
  }
};

class ContractEmployee : public Employee {
  double contractAmount;

public:
  ContractEmployee(int id, string n, double c)
      : Employee(id, n), contractAmount(c) {}

  void calculateSalary() override {
    cout << "Salary: " << contractAmount << endl;
  }
};

int main() {

  Employee *e;
  int choice;

  cout << "1. Full-Time\n2. Part-Time\n3. Contract\n";
  cout << "Enter choice: ";
  cin >> choice;

  if (choice == 1)
    e = new FullTimeEmployee(1, "Ravi", 30000, 5000);
  else if (choice == 2)
    e = new PartTimeEmployee(2, "Anita", 80, 300);
  else
    e = new ContractEmployee(3, "Suresh", 45000);

  e->calculateSalary();
  delete e;

  return 0;
}
