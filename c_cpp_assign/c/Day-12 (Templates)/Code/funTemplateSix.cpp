/*
 *	Template Specialization
 *
 *
 *
 */

#include <iostream>
using namespace std;

template <typename DT> 
void fun(DT var){ //generalized/generic function template
	cout<<"var: "<<var<<endl;
}
void fun(const char *cStr){	//specialized/specific function template
	cout<<"C Style: "<<cStr<<endl;
}

int main(){
	fun(10); 
	fun(234.345);
	fun("C Style String Here");
}
