/*
 *	Multiple Template parameters --> different types
 *
 *
 */

#include <iostream>
using namespace std;

template <typename DT, typename DU>
void fun(DT varOne, DU varTwo){
	cout<<"varOne: "<<varOne<<"\tvarTwo: "<<varTwo<<endl;
}

int main(){
	fun(10, 9273.35);
	fun('A', 10);
	fun(10.234f, "Hello");
	fun(10.234, 100);
	fun("C Style String", 123.34);
}
