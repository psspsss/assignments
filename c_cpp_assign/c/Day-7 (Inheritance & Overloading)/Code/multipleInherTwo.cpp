#include <iostream>
using namespace std;

class BaseOne{
public:
	void funOne(){cout <<"BaseOne::funOne()"<<endl;}
	void funTwo(){cout <<"BaseOne::funTwo()"<<endl;}
};

class BaseTwo{
public:
	void funOne(){cout <<"BaseTwo::funOne()"<<endl;}
	void funTwo(){cout <<"BaseTwo::funTwo()"<<endl;}
};

class Derived: public BaseOne, public BaseTwo{ //having 2 base classes
public:
	using BaseOne::funOne;
   	using BaseTwo::funTwo;
	void funThree() {cout <<"Derived::funThree()"<<endl; }
	void funFour() {cout <<"Derived::funFour()"<<endl; }
};

int main(){
	Derived dObj;
	dObj.funOne();
	dObj.funTwo();
	dObj.funThree();
	dObj.funFour();
}
