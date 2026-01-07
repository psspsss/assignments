#include <iostream>
using namespace std;

template<typename T>
T Add(T x, T y){
	return x+y;
}

int main(){
	cout<<"Result :"<<Add(10,20)<<endl;
	cout<<"Result :"<<Add(10.234,20.567)<<endl;
}
