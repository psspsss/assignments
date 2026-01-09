#include <iostream>
#include <thread>
#include <chrono>
#include <future>
using namespace std;

void calculateSum(promise<int> prom){
	int sum = 0;
	for (int i = 1; i <= 10; i++){
		this_thread::sleep_for(chrono::seconds(1));
		sum+=i;
	}
	prom.set_value(sum);//result is produced here
}

int main(){
	promise<int> pObj;
	future<int> fObj = pObj.get_future();

	thread t1(calculateSum, move(pObj));

	int result = fObj.get(); //wait until result is ready
	cout<<"Result :"<<result<<endl;

	t1.join();
}
