#include <iostream>
using namespace std;

class Base{ //Parent
public:
	void funOne(){cout <<"Base::funOne()"<<endl;}
	void funTwo(){cout <<"Base::funTwo()"<<endl;}
};

class Derived: public Base{ //Child
public:
	void funThree() {cout <<"Derived::funThree"<<endl; }
	void funFour() {cout <<"Derived::funFour"<<endl; }
};

class DerivedOne: public Derived{//Grand-child
public:
	void funFive() {cout<<"DerivedOne::funFive"<<endl; }
	void funSix() {cout<<"DerivedOne::funSix"<<endl; }
};

int main(){
	DerivedOne dObj;
	dObj.funOne();
	dObj.funTwo();
	dObj.funThree();
	dObj.funFour();
	dObj.funFive();
	dObj.funSix();
}
