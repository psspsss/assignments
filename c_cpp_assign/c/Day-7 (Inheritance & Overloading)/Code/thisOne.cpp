#include <iostream>
using namespace std;

class Test{
public:
	void funOne(){
		cout<<"in funOne "<<this<<endl;
	}
};

int main(){
	Test objOne;
	Test objTwo;
	cout <<"In main() objOne --> "<<&objOne<<endl;	
	cout <<"In main() objTwo --> "<<&objTwo<<endl;	
	objOne.funOne();//funOne(&objOne)
	objTwo.funOne();//funOne(&objTwo)
}

