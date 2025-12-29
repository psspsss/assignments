// Purpose: Demonstrate unsigned integer countdown and underflow with a while loop
// Date: 19/12/2025
// Author: Vinayak Majhi
// ID: 58618
// Batch ID: 25SUB4505 - DATACOM+5G

#include <stdio.h>
int main(){
	unsigned int cnt = 10;
	while (cnt >= 0){
		printf("%u  ", cnt);
		cnt--;
	}
	printf("\n");
}

