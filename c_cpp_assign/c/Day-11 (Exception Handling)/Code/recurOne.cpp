/*
 *	Unwinding of stack...
 *		During unwinding of stack, the destructors of local(stack
 *		based) objects will be called.
 *
 */

#include <iostream>
using namespace std;

struct Test{
	int data;
	Test(int x):data(x){cout<<"Test("<<data<<")"<<endl; }
	~Test(){cout<<"~Test()"<<endl; }
};

void recur(int);

int main(){
	try{
		recur(1);
		cout<<endl;
	}catch(int num){//handling the exception
		cout<<"In main() Caught "<<num<<endl;
	}
}

void recur(int num){
	Test *ptr = new Test(num); //heap based object

	if(num <= 5){
		cout<<num<<" ";
		recur(num + 1);
		cout<<num<<" ";
	}
	throw num;//exception is raised
	delete ptr; //deleting 
}

