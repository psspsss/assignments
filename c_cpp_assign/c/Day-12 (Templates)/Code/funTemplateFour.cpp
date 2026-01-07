#include <iostream>
using namespace std;

template<typename T>
T Add(T x, T y){
	return x+y;
}

template<typename T>
T Add(T x, T y, T z){
	return x+y+z;
}

int main(){
	cout<<"Result :"<<Add(10,20)<<endl;
	cout<<"Result :"<<Add(10.234,20.567)<<endl;

	cout<<"Result :"<<Add(10.234,20.567,834.34)<<endl;
	cout<<"Result :"<<Add(10,20,30)<<endl;
}
