#include <iostream>
#include <memory>
using namespace std;

struct B;

struct A{
	shared_ptr<B> b;
	~A(){cout<<"~A()"<<endl; }
};

struct B{
	shared_ptr<A> a;
	~B(){cout<<"~B()"<<endl; }
};


int main(){
	shared_ptr<A> objA(new A());
	shared_ptr<B> objB(new B());

	objA->b = objB;
	objB->a = objA;
}//destructors are not called
