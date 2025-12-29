// Purpose: Show static global array allocation to illustrate memory segment usage
// Date: 24/12/2025
// Author: Vinayak Majhi
// ID: 58618
// Batch ID: 25SUB4505 - DATACOM+5G

#include <iostream>
using namespace std;

int arr[1024]; // = {1, 2, 3, 4};
int main() { arr[5] = 50; }
