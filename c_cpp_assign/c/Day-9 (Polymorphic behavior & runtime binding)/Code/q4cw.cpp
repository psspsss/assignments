// Purpose: Array Operations System using abstract base class
// Date: 02/01/2026
// Author: Vinayak Majhi
// ID: 58618
// Batch ID: 25SUB4505 - DATACOM+5G

#include <iostream>
using namespace std;

class ArrayBase {
public:
  virtual void insert() = 0;
  virtual void display() = 0;
  virtual ~ArrayBase() {}
};

class IntegerArray : public ArrayBase {
  int arr[5];

public:
  void insert() override {
    cout << "Enter 5 integers:\n";
    for (int i = 0; i < 5; i++)
      cin >> arr[i];
  }

  void display() override {
    cout << "Integer Array: ";
    for (int i = 0; i < 5; i++)
      cout << arr[i] << " ";
    cout << endl;
  }
};

class FloatArray : public ArrayBase {
  float arr[5];

public:
  void insert() override {
    cout << "Enter 5 floats:\n";
    for (int i = 0; i < 5; i++)
      cin >> arr[i];
  }

  void display() override {
    cout << "Float Array: ";
    for (int i = 0; i < 5; i++)
      cout << arr[i] << " ";
    cout << endl;
  }
};

class StringArray : public ArrayBase {
  string arr[5];

public:
  void insert() override {
    cout << "Enter 5 strings:\n";
    for (int i = 0; i < 5; i++)
      cin >> arr[i];
  }

  void display() override {
    cout << "String Array: ";
    for (int i = 0; i < 5; i++)
      cout << arr[i] << " ";
    cout << endl;
  }
};

int main() {

  ArrayBase *a;
  int choice;

  cout << "1. Integer Array\n2. Float Array\n3. String Array\n";
  cout << "Enter choice: ";
  cin >> choice;

  if (choice == 1)
    a = new IntegerArray();
  else if (choice == 2)
    a = new FloatArray();
  else
    a = new StringArray();

  a->insert();
  a->display();

  delete a;
  return 0;
}
