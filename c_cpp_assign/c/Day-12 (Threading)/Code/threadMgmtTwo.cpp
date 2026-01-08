#include <iostream>
#include <thread>

void funOne(){
	for (int cnt=0;cnt<100;cnt++){
		std::cout<<"0";
	}
}

int main(){
	std::thread t1(funOne); //thread object t1 is created.
	
	t1.detach(); //waiting for thread t1 to complete
	for (int cnt=0;cnt<100;cnt++){
		std::cout<<"X";
	}
}

