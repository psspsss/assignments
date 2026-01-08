#include <iostream>
#include <thread>
using namespace std;

void funOne(int arg){
	std::cout<<"New Thread: "<<arg<<endl;
}

int main(){
	std::thread t1(funOne, 100); 
	
	std::cout<<"Main Thread: "<<endl;
	t1.join(); 
}

