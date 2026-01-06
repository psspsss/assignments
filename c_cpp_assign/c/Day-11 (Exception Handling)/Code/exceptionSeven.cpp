#include <iostream>
#include <cstdlib>
#include <exception>
using namespace std;

void funOne(){
	cout<<"void funOne() called..."<<endl;
	exit(1);//quitting before abort() is called
}

int main(){
	set_terminate(funOne);

	cout<<"Outer statement #1"<<endl;
	try {
		cout<<"Inner statement #1"<<endl;
		//throw 10;
		cout<<"statement #2"<<endl;
		//throw 104.34;
		cout<<"statement #3"<<endl;
		throw 104.34f;
		cout<<"statement #4"<<endl;
	}catch(int x){
		cout<<"Int type exception: "<<x<<endl;
	}catch(double x){
		cout<<"Double type exception: "<<x<<endl;
	}
	cout<<"Outer statement #5"<<endl;
}
