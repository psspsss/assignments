#include <iostream>
#include <thread>

void funOne(){
	while (1){
		std::cout<<"0";
	}
}

int main(){
	std::thread t1(funOne); //thread object t1 is created.

	while(1){
		std::cout<<"X";
	}
}

