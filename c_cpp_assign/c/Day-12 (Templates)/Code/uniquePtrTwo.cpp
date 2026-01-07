#include <memory>
#include <iostream>
using namespace std;

int main(){
	unique_ptr<int> ptrOne(new int(100));

	cout<<"*ptrOne: "<<*ptrOne<<endl;
	unique_ptr<int> ptrTwo = move(ptrOne); //moving of resource done here 

	if (! ptrOne)
		cout<<"ptrOne is null after the move"<<endl;
	cout<<"*ptrTwo: "<<*ptrTwo<<endl;
}
