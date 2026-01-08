#include <iostream>
#include <thread>
using namespace std;

int globVar = 0;

void Update(){
	for (int cnt=0;cnt < 100000; cnt++)
		globVar++;
}


int main(){
	thread t1(Update);
	thread t2(Update);

	for (int cnt=0;cnt < 100000; cnt++)
		globVar++;

	t1.join();
	t2.join();

	cout<<"global var: "<<globVar<<endl;
}
