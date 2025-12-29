#include <iostream>
using namespace std;

class Employee {
  static string compName; // class member declaration
public:
  static string company() { return compName; } // class member
};

int main() {
  cout << "Employee::company() --> " << Employee::company() << endl;
  Employee obj1;
  cout << "obj1.company() --> " << obj1.company() << endl;
}

string Employee::compName = "Wipro Ltd";
