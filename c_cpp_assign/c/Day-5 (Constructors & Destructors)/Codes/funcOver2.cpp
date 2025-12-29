// Purpose: Show overload resolution between int and float when called with a float literal
// Date: 26/12/2025
// Author: Vinayak Majhi
// ID: 58618
// Batch ID: 25SUB4505 - DATACOM+5G

#include <iostream>
using namespace std;

void fun(int);
void fun(float);

int main() { fun(10.234); }

void fun(int arg) { cout << "Int type: arg  " << arg << endl; }
void fun(float arg) { cout << "float type: arg  " << arg << endl; }
