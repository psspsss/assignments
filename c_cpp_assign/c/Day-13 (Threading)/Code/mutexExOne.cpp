#include <iostream>
#include <thread>
#include <mutex>
using namespace std;

int globVar = 0;
mutex mVar;

void Update(){
	mVar.lock(); //resource is locked
	for (int cnt=0;cnt < 100000; cnt++)
		globVar++;
	mVar.unlock();//resource is unlocked
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
