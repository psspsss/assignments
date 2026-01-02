
// Purpose: Student Management System using inheritance and polymorphism
// Date: 02/01/2026
// Author: Vinayak Majhi
// ID: 58618
// Batch ID: 25SUB4505 - DATACOM+5G

#include <iostream>
using namespace std;

class Student {
protected:
  string name;
  int rollNo;

public:
  Student(string n, int r) : name(n), rollNo(r) {}

  virtual void calculateGrade() = 0;

  virtual ~Student() {}
};

class Undergraduate : public Student {
  int marks;

public:
  Undergraduate(string n, int r, int m) : Student(n, r), marks(m) {}

  void calculateGrade() override {
    cout << name << " (Undergraduate): " << (marks >= 50 ? "Pass" : "Fail")
         << endl;
  }
};

class Postgraduate : public Student {
  int marks;

public:
  Postgraduate(string n, int r, int m) : Student(n, r), marks(m) {}

  void calculateGrade() override {
    cout << name << " (Postgraduate): " << (marks >= 60 ? "Pass" : "Fail")
         << endl;
  }
};

class ResearchStudent : public Student {
  int publications;

public:
  ResearchStudent(string n, int r, int p) : Student(n, r), publications(p) {}

  void calculateGrade() override {
    cout << name << " (Research): "
         << (publications >= 2 ? "Satisfactory" : "Unsatisfactory") << endl;
  }
};

int main() {

  Student *s;
  int choice;

  cout << "1. Undergraduate\n2. Postgraduate\n3. Research\n";
  cout << "Enter choice: ";
  cin >> choice;

  if (choice == 1)
    s = new Undergraduate("Amit", 101, 65);
  else if (choice == 2)
    s = new Postgraduate("Neha", 201, 58);
  else
    s = new ResearchStudent("Rahul", 301, 3);

  s->calculateGrade();
  delete s;

  return 0;
}
