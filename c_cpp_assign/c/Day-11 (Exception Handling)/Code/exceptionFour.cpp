#include <iostream>
using namespace std;

//dynamic array
void createArray(int);

int main(){ 
	createArray(10);
	createArray(20);
	createArray(-120);
	createArray(200);
	cout<<"Executes normally even if exception occurs..."<<endl;
}
void createArray(int size){
	try{
		if (size<=0)
			throw size; //raising an exception
		int *arr = new int [size];
		for (int cnt=0;cnt<size;cnt++)
			arr[cnt] = 1001 + cnt; 

		cout<<"Array: ";
		for (int cnt=0;cnt<size;cnt++) 
			cout<<arr[cnt]<<" ";
		cout<<"\n************************************************\n";

		delete []arr; 
	}catch(int var){//handling the exception
		cout<<"Caught int type Exception "<<var<<endl;
	}

}
