#include <iostream>
using namespace std;

int main(){
	int var = 100; //local variable 
	auto myLambda = [&var] {
		cout<<"Hello Welcome to Lambdas in C++: "<<var<<endl; 
		var+=10;
	};

	myLambda();
	cout<<"after updation in Main(): "<<var<<endl; 
}
