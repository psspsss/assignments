#include <iostream>
using namespace std;
//code has an issue - dangling pointer
class MyArray{ 
	int *arr; 
	int size; 
public:
	MyArray();
	MyArray(int data);
	MyArray(const MyArray& );
	void printArray();
	
	~MyArray(){
		if (size){
			delete [] arr;
		}
		size=0;
	}
};

int main(){
	MyArray objTwo(100);//single parameterized ctor
	objTwo.printArray();
	{
		MyArray objThree = objTwo;//copying one object to another
		objThree.printArray();
	}
	objTwo.printArray();
}

MyArray::MyArray(const MyArray& rhs){
	size = rhs.size;
	arr = new int[size]; //separate memory allocation 
	for (int cnt = 0;cnt < size; cnt++)
		arr[cnt] = rhs.arr[cnt]; //data being copied
}
MyArray::MyArray():size(10){
	arr = new int[size];
	for (int cnt=0;cnt< size; cnt++)
		arr[cnt] = 0;
}

MyArray::MyArray(int sz):size(sz) {
	int data = 101;	
	arr = new int[size];
	for (int cnt=0;cnt< size; cnt++)
		arr[cnt] = data + cnt;
}

void MyArray::printArray(){
	cout<<"Size: "<<size<<"\tarray: "<<endl;
	for(int cnt=0;cnt < size; cnt++)
		cout<<arr[cnt]<<"  ";
	cout<<endl;
}
