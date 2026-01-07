#include <memory>
#include <iostream>
using namespace std;

class Test{
	int data;
public:
	Test(int num = 0):data(num) {cout<<"Test()"<<endl;}
	~Test(){cout<<"~Test()"<<endl;}
	void disp(){cout<<"Test::disp(): "<<data<<endl; }
};

int main(){
	//unique_ptr<Test> ptrOne= make_unique<Test>(100);
	unique_ptr<Test> ptrOne(new Test(100));
	ptrOne->disp();
	cout<<"In main()"<<endl;
}
