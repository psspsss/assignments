#include <iostream>
using namespace std;

int fun();

int main(){
	cout<<"fun() "<<fun()<<endl;	
	cout<<"fun() "<<fun()<<endl;	
	cout<<"fun() "<<fun()<<endl;	
	cout<<"fun() "<<fun()<<endl;	
}

int fun(){
	static int var = 10;
	return var++;
}
