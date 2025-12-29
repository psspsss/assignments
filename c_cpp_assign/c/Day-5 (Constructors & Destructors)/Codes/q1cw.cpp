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
