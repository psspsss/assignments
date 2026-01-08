/*
 *	Concurrent execution
 *
 */

#include <iostream>
#include <thread>
#include <chrono>
using namespace std;

void fun(){
	cout<<"fun() called"<<endl; 
	this_thread::sleep_for(chrono::seconds(1));
}

int main(){
	thread t1(fun);
	thread t2(fun);
	thread t3(fun);
	thread t4(fun);
	thread t5(fun);

	t1.join(); 
	t2.join(); 
	t3.join(); 
	t4.join(); 
	t5.join(); 
}
