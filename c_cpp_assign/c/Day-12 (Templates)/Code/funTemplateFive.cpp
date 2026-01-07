
#include <iostream>
using namespace std;

template <typename DT> 
void fun(DT var){
	cout<<"var: "<<var<<endl;
}

int main(){
	fun(10); //Implicit call  
	fun<int>(10); //Explicit call fun<datatype>(arg); 
			//datatype is passed as an argument along with actual
			//arguments. Datatype is passed in <data type> (actual args)

	fun<double>(234.345);
	fun(234.345);
}
