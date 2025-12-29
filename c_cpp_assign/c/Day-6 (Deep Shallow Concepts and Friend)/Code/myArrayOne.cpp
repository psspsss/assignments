#include <iostream>
using namespace std;

class MyArray{ //abstraction
	int arr[100];
	int size;
public:
	MyArray();
	MyArray(int data);
	MyArray(const MyArray&);
	void printArray();
	MyArray& extend(MyArray&);
};


int main(){
	MyArray objOne;
	MyArray objTwo(100);
	MyArray objThree = objTwo;
	objOne.printArray();
	objTwo.printArray();
	objThree.printArray();

	//objFour = objTwo.extend(objThree);
	//objFour.printArray();
}

MyArray::MyArray(){
	size = 100;
	for (int cnt=0;cnt< size; cnt++)
		arr[cnt] = 0;
}

MyArray::MyArray(int data){
	size = 100;
	for (int cnt=0;cnt< size; cnt++)
		arr[cnt] = data + cnt;
}


MyArray::MyArray(const MyArray& rhsObj){
	size = rhsObj.size;
	for (int cnt=0;cnt< size; cnt++)
		arr[cnt] = rhsObj.arr[cnt];

}
void MyArray::printArray(){
	cout<<"Size: "<<size<<"\tarray: "<<endl;
	for(int cnt=0;cnt < size; cnt++)
		cout<<arr[cnt]<<"  ";
	cout<<endl;
}

MyArray& MyArray::extend(MyArray& rhsObject){
	//For future implementation with dynamic memory allocation
	
	return rhsObject;
}

