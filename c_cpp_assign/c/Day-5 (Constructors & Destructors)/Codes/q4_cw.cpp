#include <iostream>

using namespace std;

void printcalendar(int day = 1, int month = 12, int year = 2025);

int main() { printcalendar(12); }

void printcalendar(int day, int month, int year) {
  cout << "Date: " << day << " : " << month << " : " << year << endl;
}
