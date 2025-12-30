#include <iostream>
using namespace std;

class Test{
public:
	Test& funOne(){
		cout<<"in funOne "<<this<<endl;
		return *this;
	}
	void funTwo(){
		cout<<"in funTwo"<<endl;
	}
};

int main(){
	Test objOne;
	/*
	Test& temp = objOne.funOne();//this will return objOne's reference
	temp.funTwo();//using temp's reference used to call funTwo()
	*/
	objOne.funOne().funTwo();
}

