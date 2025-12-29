
#include <iostream>
using namespace std;

class Calendar {
  int day, month, year;

public:
  void setDate(int dd = 1, int mm = 12, int yyyy = 2025) {
    day = dd;
    month = mm;
    year = yyyy;
  }

  bool isLeap() {
    if ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0))
      return true;
    return false;
  }

  int maxDays() {
    if (month == 2)
      return isLeap() ? 29 : 28;
    if (month == 4 || month == 6 || month == 9 || month == 11)
      return 30;
    return 31;
  }

  bool valideDate() {
    if (month < 1 || month > 12)
      return false;
    if (day < 1 || day > maxDays())
      return false;
    return true;
  }

  int dayOfWeek() {
    int d = day, m = month, y = year;
    if (m < 3) {
      m += 12;
      y--;
    }
    int k = y % 100;
    int j = y / 100;
    return (d + (13 * (m + 1)) / 5 + k + k / 4 + j / 4 + 5 * j) % 7;
  }

  void printCalendar() {
    cout << "Date: " << day << "/" << month << "/" << year << endl;
  }
};

int main() {
  Calendar c;
  c.setDate(26, 12, 2025);

  if (c.valideDate()) {
    c.printCalendar();
    cout << "Day of Week (0=Sat): " << c.dayOfWeek() << endl;
  } else {
    cout << "Invalid Date" << endl;
  }

  return 0;
}
