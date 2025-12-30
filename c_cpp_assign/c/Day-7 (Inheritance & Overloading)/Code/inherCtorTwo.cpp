#include <iostream>
using namespace std;

class Base{
public:
	Base() {cout<<"Base::Base()"<<endl; }
	Base(int) {cout<<"Base::Base(int)"<<endl; }
	~Base() {cout<<"Base::~Base()"<<endl; }
};

class Derived: public Base{
public:
	Derived() {cout<<"Derived::Derived()"<<endl; }
	Derived(int x):Base(x) {cout<<"Derived::Derived("<<x<<")"<<endl; }
	~Derived() {cout<<"Derived::~Derived()"<<endl; }
};

int main(){
	Derived dObj=100;
}
