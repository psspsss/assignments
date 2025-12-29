// Purpose: Demonstrate default function parameters by printing a date with defaults
// Date: 26/12/2025
// Author: Vinayak Majhi
// ID: 58618
// Batch ID: 25SUB4505 - DATACOM+5G

#include <iostream>

using namespace std;

void printcalendar(int day = 1, int month = 12, int year = 2025);

int main() { printcalendar(12); }

void printcalendar(int day, int month, int year) {
  cout << "Date: " << day << " : " << month << " : " << year << endl;
}
