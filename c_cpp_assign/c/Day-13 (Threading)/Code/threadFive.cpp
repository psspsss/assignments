#include <iostream>
#include <thread>
class Test{
public:
	void operator()(){
		std::cout<<"Thread from Test"<<std::endl;
	}
};

int main(){
	Test obj = Test();
	std::thread t1(obj); //thread object t1 is created.

	std::cout<<"Thread from main()"<<std::endl;

	t1.join();
}

