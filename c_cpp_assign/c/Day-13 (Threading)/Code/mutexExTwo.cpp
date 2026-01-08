/*
 *	If lock() is established, and if not unlocked()
 *		then, it results in deadlock --> your program will not
 *		continue further
 */

#include <iostream>
#include <thread>
#include <mutex>
using namespace std;

int globVar = 0;
mutex mVar; // this is an important resource 
	//It has to be wrapped around in a class 
	//RAII 

void Update(){
	mVar.lock(); //resource is locked
	for (int cnt=0;cnt < 100000; cnt++)
		globVar++;
	mVar.unlock(); //resource is locked
}


int main(){
	thread t1(Update);
	thread t2(Update);

	mVar.lock();
	for (int cnt=0;cnt < 100000; cnt++)
		globVar++;
	mVar.unlock();

	t1.join();
	t2.join();

	cout<<"global var: "<<globVar<<endl;
}
