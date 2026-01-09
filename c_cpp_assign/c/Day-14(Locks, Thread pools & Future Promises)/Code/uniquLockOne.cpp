#include <iostream>
#include <thread>
#include <mutex>
using namespace std;

int globVar = 0;
mutex mVar;

void Update(){
	unique_lock<mutex> lock(mVar);
	for (int cnt=0;cnt < 100000; cnt++)
		globVar++;
	lock.unlock();
}

int main(){
	thread t1(Update);
	thread t2(Update);
	thread t3(Update);

	t1.join();
	t2.join();
	t3.join();

	cout<<"global var: "<<globVar<<endl;
}

