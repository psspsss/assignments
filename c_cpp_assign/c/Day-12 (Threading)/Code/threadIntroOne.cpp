/*
 *	Sequential execution
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
	fun();
	fun();
	fun();
	fun();
	fun();
}
