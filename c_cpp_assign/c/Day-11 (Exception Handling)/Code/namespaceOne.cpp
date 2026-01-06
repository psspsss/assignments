#include <iostream>
using namespace std;

namespace Mine{
	int var = 100;
	void funOne(){
		cout<<"namespace Mine::funOne()"<<endl;
	}
};

namespace Yours{
	int var = 200;
	void funOne(){
		cout<<"namespace Yours::funOne()"<<endl;
	}
};

namespace Ours{
	int var = 300;
	void funOne(){
		cout<<"namespace Ours::funOne()"<<endl;
	}
};

int main(){
	cout<<"Mine::var "<<Mine::var<<endl;
	cout<<"Yours::var "<<Yours::var<<endl;
	cout<<"Ours::var "<<Ours::var<<endl;
	Mine::funOne();
	Yours::funOne();
	Ours::funOne();
}



