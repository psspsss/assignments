#include <iostream>
using namespace std;

class Test{
	int data;
public:
	void funOne(int data){
		this->data = data;
		cout<<"funOne Data: "<<data<<endl;
	}
	void funTwo(){
		cout<<"funTwo Data: "<<data<<endl;
	}
};

int main(){
	Test obj;
	obj.funOne(10);
	obj.funTwo();
}
