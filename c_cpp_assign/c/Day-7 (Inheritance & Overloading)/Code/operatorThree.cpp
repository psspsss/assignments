#include <iostream>
using namespace std;

class Num{
	int data;
public:
	Num(int x=0): data(x) {}
	Num operator+(const Num& rhs){
		Num temp(data + rhs.data);
		return temp;
	}

	Num operator-(const Num& rhs){
		Num temp(data - rhs.data);
		return temp;
	}
	friend ostream& operator<<(ostream &, const Num &);
};

ostream& operator<<(ostream &out, const Num &obj){
	out<<"data: "<<obj.data;
	return out;
}

int main(){
	Num a = 100, b = 20;
	cout<<a<<"   "<<b<<endl;
	Num c = a + b;
	Num d = c + a - b;
	cout<<c<<"   "<<d<<endl;
}


