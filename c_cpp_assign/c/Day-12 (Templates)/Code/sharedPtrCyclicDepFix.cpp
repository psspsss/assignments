#include <iostream>
#include <memory>
using namespace std;

struct B;

struct A{
	shared_ptr<B> b;
	~A(){cout<<"~A()"<<endl; }
};

struct B{
	weak_ptr<A> a;// weak pointer
	~B(){cout<<"~B()"<<endl; }
};


int main(){
	shared_ptr<A> objA(new A());
	shared_ptr<B> objB(new B());

	objA->b = objB;
	objB->a = objA;
}//destructors are not called
