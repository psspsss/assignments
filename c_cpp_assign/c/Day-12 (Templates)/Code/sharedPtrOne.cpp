#include <iostream>
#include <memory>
using namespace std;

int main(){
	shared_ptr<int> ptrOne(new int(100));

	cout<<"1 Use count/Reference Count: "<<ptrOne.use_count()<<endl;

	{
			shared_ptr<int> ptrTwo = ptrOne; //local temp object

			cout<<"2. Use count/Reference Count: "<<*ptrOne<<"\t"<<ptrOne.use_count()<<endl;
			cout<<"2. Use count/Reference Count: "<<*ptrTwo<<"\t"<<ptrTwo.use_count()<<endl;
	}
	cout<<"3 Use count/Reference Count: "<<ptrOne.use_count()<<endl;
}

