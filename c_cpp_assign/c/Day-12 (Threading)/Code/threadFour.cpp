#include <iostream>
#include <thread>

int main(){
	std::thread t1([] {
		std::cout<<"Inside lambda here"<<std::endl;				
	}); //thread object t1 is created by passing lambda expression.

	std::cout<<"In main()"<<std::endl;

	t1.join();
}

