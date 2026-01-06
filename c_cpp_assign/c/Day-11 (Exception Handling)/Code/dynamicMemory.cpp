// Purpose: MyString class with dynamic memory and exception handling
// Date: 06/01/2026
// Author: Vinayak Majhi
// ID: 58618
// Batch ID: 25SUB4505 - DATACOM+5G

#include <cstring>
#include <exception>
#include <iostream>

using namespace std;

class MyString {
  char *str;
  int len;

public:
  MyString(int l) {
    if (l <= 0)
      throw invalid_argument("Invalid string length");

    len = l;
    str = new char[len + 1];
    str[0] = '\0';
  }

  void input() {
    cout << "Enter string (max " << len << " chars): ";
    cin.getline(str, len + 1);
  }

  void display() const {
    cout << "String: " << str << endl;
    cout << "Length: " << strlen(str) << endl;
  }

  ~MyString() { delete[] str; }
};

int main() {
  try {
    int size;
    cout << "Enter string size: ";
    cin >> size;
    cin.ignore();

    MyString s(size);
    s.input();
    s.display();
  } catch (exception &e) {
    cout << "Exception caught: " << e.what() << endl;
  }

  return 0;
}
