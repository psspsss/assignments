// Purpose: Demonstrate diamond problem without virtual inheritance
// Date: 29/12/2025
// Author: Vinayak Majhi
// ID: 58618
// Batch ID: 25SUB4505 - DATACOM+5G

#include <iostream>
using namespace std;

class Person {
public:
  string name;

  Person(string n) : name(n) {
    cout << "Person Constructor (" << name << ")" << endl;
  }
};

class Student : public Person {
public:
  int rollNo;

  Student(string n, int r) : Person(n), rollNo(r) {
    cout << "Student Constructor" << endl;
  }
};

class Employee : public Person {
public:
  int empId;

  Employee(string n, int e) : Person(n), empId(e) {
    cout << "Employee Constructor" << endl;
  }
};

class Intern : public Student, public Employee {
public:
  Intern(string n, int r, int e) : Student(n, r), Employee(n, e) {
    cout << "Intern Constructor" << endl;
  }
};

int main() {

  Intern i("Vinayak", 101, 5001);

  // Ambiguous access (compile-time error)
  // cout << i.name;

  // Unambiguous access
  cout << "Student::Person name = " << i.Student::name << endl;
  cout << "Employee::Person name = " << i.Employee::name << endl;

  cout << "\nObject Sizes:" << endl;
  cout << "Person: " << sizeof(Person) << endl;
  cout << "Student: " << sizeof(Student) << endl;
  cout << "Employee: " << sizeof(Employee) << endl;
  cout << "Intern: " << sizeof(Intern) << endl;

  return 0;
}
