#include <iostream>
#include <thread>

void funOne(){
	std::cout<<"Thread here "<<std::endl;
}

int main(){
	std::thread t1(funOne); //thread object t1 is created.
	std::cout<<"In main()"<<std::endl;

	t1.join();
}

