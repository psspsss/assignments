// Purpose: Demonstrate runtime polymorphism using Shape hierarchy
// Date: 31/12/2025
// Author: Vinayak Majhi
// ID: 58618
// Batch ID: 25SUB4505 - DATACOM+5G

#include <iostream>
using namespace std;

class Shape {
public:
  virtual void draw() { cout << "Drawing Shape" << endl; }

  virtual ~Shape() {}
};

class Circle : public Shape {
public:
  void draw() override { cout << "Drawing Circle" << endl; }
};

class Rectangle : public Shape {
public:
  void draw() override { cout << "Drawing Rectangle" << endl; }
};

class Triangle : public Shape {
public:
  void draw() override { cout << "Drawing Triangle" << endl; }
};

int main() {

  Shape *s;

  Circle c;
  Rectangle r;
  Triangle t;

  s = &c;
  s->draw();

  s = &r;
  s->draw();

  s = &t;
  s->draw();

  return 0;
}
