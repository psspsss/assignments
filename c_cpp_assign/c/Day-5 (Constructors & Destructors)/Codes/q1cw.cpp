// Purpose: Calendar function example demonstrating default parameters and printing a date
// Date: 26/12/2025
// Author: Vinayak Majhi
// ID: 58618
// Batch ID: 25SUB4505 - DATACOM+5G

#include <iostream>

using namespace std;

void area(int, int);
void area(int);
void area(float);

int main() {

  area(2, 4);
  area(40);
  area(5.16f);
}

void area(int lenght, int breadth) {

  int result = lenght * breadth;

  cout << "Area of Rectangle = " << result << endl;
}

void area(int side) {
  int result = side * side;
  cout << "Area of Square = " << result << endl;
}

void area(float radius) {
  float result = 3.14 * radius * radius;

  cout << "Area of Circle = " << result << endl;
}
