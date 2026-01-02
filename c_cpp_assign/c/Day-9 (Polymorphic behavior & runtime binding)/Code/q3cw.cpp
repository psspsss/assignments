// Purpose: Student Attendance System using inheritance and polymorphism
// Date: 02/01/2026
// Author: Vinayak Majhi
// ID: 58618
// Batch ID: 25SUB4505 - DATACOM+5G

#include <iostream>
using namespace std;

class StudentAttendance {
protected:
  string name;
  int attendance;

public:
  StudentAttendance(string n, int a) : name(n), attendance(a) {}

  virtual bool checkEligibility() = 0;
  string getName() const { return name; }

  virtual ~StudentAttendance() {}
};

class EngineeringStudent : public StudentAttendance {
public:
  EngineeringStudent(string n, int a) : StudentAttendance(n, a) {}

  bool checkEligibility() override { return attendance >= 75; }
};

class ArtsStudent : public StudentAttendance {
public:
  ArtsStudent(string n, int a) : StudentAttendance(n, a) {}

  bool checkEligibility() override { return attendance >= 65; }
};

class ScienceStudent : public StudentAttendance {
public:
  ScienceStudent(string n, int a) : StudentAttendance(n, a) {}

  bool checkEligibility() override { return attendance >= 70; }
};

int main() {

  StudentAttendance *sArr[] = {new EngineeringStudent("Rahul", 78),
                               new ArtsStudent("Balu", 64),
                               new ScienceStudent("Sharma", 72), NULL};

  for (int i = 0; sArr[i] != NULL; i++) {
    cout << sArr[i]->getName() << " : ";
    if (sArr[i]->checkEligibility())
      cout << "Eligible\n";
    else
      cout << "Not Eligible\n";
    delete sArr[i];
  }

  return 0;
}
