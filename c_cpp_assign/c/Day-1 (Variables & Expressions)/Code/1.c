// Purpose: Check if a specific bit is set in an integer using bitwise operators
// Date: 19/12/2025
// Author: Vinayak Majhi
// ID: 58618
// Batch ID: 25SUB4505 - DATACOM+5G

#include <stdio.h>

int main(){
	int num, pos;
	num = 10, pos = 2;
	int res = (num & 1 << pos);
	printf("%d pos in %d is %s\n", pos, num, (res ? "ON": "OFF"));
}
