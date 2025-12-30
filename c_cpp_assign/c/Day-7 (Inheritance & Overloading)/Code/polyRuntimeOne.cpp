#include <iostream>
using namespace std;

class Base{
public:
	virtual void funOne(){cout <<"Base::funOne()"<<endl; }	
};

class Derived: public Base{
public:
	void funOne() {cout<<"Derived::funOne()"<<endl;}
};

int main(){
	//Remember Derived is a Base class
	Base *ptr, bObj;
	Derived dObj;

	ptr = &bObj; //Base class object's address is assigned to Base class Pointer
	ptr->funOne();

	ptr = &dObj;//Derived class Object's address is assigned to Derived Class Pointer 
	ptr->funOne();
}

