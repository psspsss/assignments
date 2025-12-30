#include <iostream>
using namespace std;

class Num{
	int data;
public:
	Num(int x=0): data(x) {}
	Num& operator++(){
		cout<<"Pre-fix"<<endl;
		data++;
		return *this;
	}
	Num operator++(int){
		cout<<"Post-fix"<<endl;
		Num temp(*this);//store previous data here 
		data++; 
		return temp; //return non-modified data
	}

	void disp(ostream &out){
		out<<"data: "<<data<<endl;
	}
};

int main(){
	Num a = 10;
	++a;
	a.disp(cout);
	a++;
	a.disp(cout);
}


