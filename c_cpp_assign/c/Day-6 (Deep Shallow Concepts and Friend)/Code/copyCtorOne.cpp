#include <iostream>
using namespace std;

class Test{
	public:
		Test(){cout<<"Test()--> Default Ctor"<<endl; }
		Test(int){cout<<"Test(int)--> Parameterized Ctor"<<endl; }

		~Test(){cout<<"~Test()--> Destructor"<<endl; }
		Test(const Test &) {cout<<"Test(const Test&) ---> Copy Ctor"<<endl;}
};


int main(){
	Test objOne ;//single parameterized ctor
	Test objTwo = 100;//single parameterized ctor
	Test objThree = objTwo;//single parameterized ctor
}

