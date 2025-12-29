#include <iostream>
using namespace std;

class Base{
public:
	void funOne(){cout<<"Base::funOne()"<<endl; }
	void funTwo(){cout<<"Base::funTwo()"<<endl; }
};

class Derived:public Base{};

int main(){
	Derived dObj;
	dObj.funOne();
	dObj.funTwo();
}
