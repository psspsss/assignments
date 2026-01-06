#include <iostream>
#include <cstdlib>
#include <exception>
using namespace std;

int main(){

	cout<<"Outer statement #1"<<endl;
	try {
		cout<<"Inner statement #1"<<endl;
		throw invalid_argument("Message here");
		cout<<"statement #2"<<endl;
	}catch(invalid_argument &e){
		cout<<"Derived: "<<e.what()<<endl;
	}catch(exception &e){
		cout<<"Base class: "<<e.what()<<endl;
	}
	cout<<"Outer statement #"<<endl;
}
