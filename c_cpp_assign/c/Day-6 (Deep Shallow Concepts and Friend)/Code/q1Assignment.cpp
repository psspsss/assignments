// Purpose: Custom MyString class with deep copy
// Date: 29/12/2025
// Author: Vinayak Majhi
// ID: 58618
// Batch ID: 25SUB4505 - DATACOM+5G

#include <cstring>
#include <iostream>

using namespace std;

class MyString {
  char *data;
  unsigned int len;

public:
  MyString() {
    len = 0;
    data = new char[1];
    data[0] = '\0';
  }

  MyString(const char *str) {
    len = strlen(str);
    data = new char[len + 1];
    strcpy(data, str);
  }

  MyString(const MyString &obj) {
    len = obj.len;
    data = new char[len + 1];
    strcpy(data, obj.data);
  }

  ~MyString() { delete[] data; }

  unsigned int size() const { return len; }

  const char *cStr() const { return data; }

  void disp() const {
    cout << "Length: " << len << endl;
    cout << "Data: " << data << endl;
  }
};

int main() {

  MyString s1("Resource");
  MyString s2 = s1;

  s1.disp();
  s2.disp();

  return 0;
}
