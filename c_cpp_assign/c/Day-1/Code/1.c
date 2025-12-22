#include <stdio.h>

int main(){
	int num, pos;
	num = 10, pos = 2;
	int res = (num & 1 << pos);
	printf("%d pos in %d is %s\n", pos, num, (res ? "ON": "OFF"));
}
