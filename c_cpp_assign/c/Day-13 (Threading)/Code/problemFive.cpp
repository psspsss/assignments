/*
 *	Creating 5 thread and joining them
 */
#include <iostream>
#include <thread>
#include <vector>
using namespace std;

void funTask(){
	cout<<"Thread created..."<<endl;
}

int main(){
	vector<thread> threadObjs;

	for(int cnt=0; cnt < 5; cnt++)
		threadObjs.emplace_back(thread(funTask));

	for (auto& tId: threadObjs)
		tId.join();
}

