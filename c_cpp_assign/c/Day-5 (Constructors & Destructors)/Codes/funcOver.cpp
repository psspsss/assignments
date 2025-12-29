// Purpose: Demonstrate function overloading with different parameter types and counts
// Date: 26/12/2025
// Author: Vinayak Majhi
// ID: 58618
// Batch ID: 25SUB4505 - DATACOM+5G

#include <iostream>
using namespace std;

void fun();
void fun(int);
void fun(double);
void fun(int, int);
void fun(double, double);

int main() {

  fun();

  fun(10);

  fun(123.345);

  fun(100, 200);

  fun(234.345, 453.234);
}

void fun() { cout << "void fun()" << endl; }
void fun(int) { cout << "void fun(int)" << endl; }
void fun(double) { cout << "void fun(double)" << endl; }
void fun(int, int) { cout << "void fun(int, int)" << endl; }
void fun(double, double) { cout << "void fun(double, double)" << endl; }
