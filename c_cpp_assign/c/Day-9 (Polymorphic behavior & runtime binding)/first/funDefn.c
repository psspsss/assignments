#include "funDecl.h"

void fun(){
	printf("void fun() called...\n");
}
void funOne(int x){
	printf("void funOne(%d) called...\n", x);
}
void funTwo(int x, int y){
	printf("void funTwo(%d, %d) called...\n", x, y);
}
void funThree(int x, int y, int z){
	printf("void funThree(%d, %d, %d) called...\n", x, y, z);
}
void funFour(){
	printf("void funFour() called...\n");
}
