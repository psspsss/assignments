// Purpose: Define Employee class with methods to input and display employee details
// Date: 26/12/2025
// Author: Vinayak Majhi
// ID: 58618
// Batch ID: 25SUB4505 - DATACOM+5G

#include <iostream>

using namespace std;

class Employee {

private:
  int id;

  char name[25];

  double sal;

public:
  void getDetails() {

    cout << "Enter ID, name and Sal: ";

    cin >> id >> name >> sal;
  }

  void dispDetails() {

    cout << "Employee Details ID: " << id << "\tName: " << name
         << "\tSal: " << sal << endl;
  }

}; // blue print of an object

int main() {

  Employee varOne;

  varOne.getDetails();

  varOne.dispDetails();
}
