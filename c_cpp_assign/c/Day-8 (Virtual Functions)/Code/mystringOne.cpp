#include <iostream>

#include <cstring>

using namespace std;

class MyString {

  char *str;

  int len;

public:
  MyString(const char *st = " ")
      : len(strlen(st) + 1) { // default constructor --> (1)

    str = new char[len + 1];

    strcpy(str, st);
  }

  ~MyString() {

    if (len)

      delete[] str;

    str = nullptr;

    len = 0;
  }

  MyString &operator=(const char *st) { // assignment operator --> (3)

    len = strlen(st) + 1;

    str = new char[len + 1];

    strcpy(str, st);

    return *this;
  }

  MyString &operator=(const MyString &rhs) { // assignment operator --> (3)

    len = rhs.len;

    str = new char[len + 1];

    strcpy(str, rhs.str);

    return *this;
  }

  MyString(const MyString &rhs) : len(rhs.len) { // Copy constructor --> (2)

    str = new char[len + 1];

    strcpy(str, rhs.str);
  }

  friend ostream &operator<<(ostream &, const MyString &);
};

int main() {

  MyString one = "One string here is to initialize";

  MyString two = one;

  MyString three;

  three = two;

  cout << "One: " << one << endl;

  cout << "Two: " << two << endl;

  cout << "Three: " << three << endl;
}

ostream &operator<<(ostream &out, const MyString &rhs) {

  out << "Len: " << rhs.len << "\t\tStr: " << rhs.str;

  return out;
}
