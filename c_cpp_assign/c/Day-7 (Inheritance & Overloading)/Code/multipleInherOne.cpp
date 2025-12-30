#include <iostream>
using namespace std;

class BaseOne{
public:
	void funOne(){cout <<"BaseOne::funOne()"<<endl;}
	void funTwo(){cout <<"BaseOne::funTwo()"<<endl;}
};

class BaseTwo{
public:
	void funThree(){cout <<"BaseTwo::funThree()"<<endl;}
	void funFour(){cout <<"BaseTwo::funFour()"<<endl;}
};

class Derived: public BaseOne, public BaseTwo{ //having 2 base classes
public:
	void funFive() {cout <<"Derived::funFive()"<<endl; }
	void funSix() {cout <<"Derived::funSix()"<<endl; }
};

int main(){
	Derived dObj;
	dObj.funOne();
	dObj.funTwo();
	dObj.funThree();
	dObj.funFour();
	dObj.funFive();
	dObj.funSix();
}
