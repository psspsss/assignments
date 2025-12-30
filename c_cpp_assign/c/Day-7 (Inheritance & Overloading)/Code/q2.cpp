// Purpose: Demonstrate operator overloading using Time class
// Date: 30/12/2025
// Author: Vinayak Majhi
// ID: 58618
// Batch ID: 25SUB4505 - DATACOM+5G

#include <iostream>

using namespace std;

class Time {
  int hrs;
  int min;

public:
  Time(int h = 0, int m = 0) {
    hrs = h;
    min = m;
    normalize();
  }

  void normalize() {
    hrs += min / 60;
    min = min % 60;
    if (min < 0) {
      min += 60;
      hrs--;
    }
  }

  Time operator+(const Time &t) {
    Time temp(hrs + t.hrs, min + t.min);
    temp.normalize();
    return temp;
  }

  Time operator-(const Time &t) {
    Time temp(hrs - t.hrs, min - t.min);
    temp.normalize();
    return temp;
  }

  friend ostream &operator<<(ostream &out, const Time &t);
};

ostream &operator<<(ostream &out, const Time &t) {
  out << t.hrs << ":";
  if (t.min < 10)
    out << "0";
  out << t.min;
  return out;
}

int main() {

  Time t1(2, 45), t2(1, 30);

  Time t3 = t1 + t2;
  Time t4 = t1 - t2;

  cout << "t1 = " << t1 << endl;
  cout << "t2 = " << t2 << endl;
  cout << "t1 + t2 = " << t3 << endl;
  cout << "t1 - t2 = " << t4 << endl;

  return 0;
}
