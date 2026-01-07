/*
 * class template:
 *		specialed class representing <const char *> and for any other
 *		data type generic class is used.
 */

#include <iostream>
#include <cstring>
using namespace std;

template <typename T>
class Sample{ //generic class
	T data;
public:
	Sample(T arg=T()):data(arg) {}
	void disp(){ cout<<"data: "<<data<<endl; }
};

template <>
class Sample<const char *>{ //specialized class
	char *data;
public:
	Sample(const char* arg) {
		data = new char[strlen(arg) + 1];
		strcpy(data, arg);
	}
	void disp(){ cout<<"const char * --->  data: "<<data<<endl; }
	~Sample(){ delete []data; }
};

int main(){
	Sample<int> iObj(100);
	Sample<double> dObj(10.234);
	Sample<const char *> sObj("Some string here");

	iObj.disp();
	dObj.disp();
	sObj.disp();
}
