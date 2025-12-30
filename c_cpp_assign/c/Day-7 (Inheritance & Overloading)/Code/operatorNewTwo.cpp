#include <iostream>
#include <cstdlib>
using namespace std;

class Test{
	int data;
public:
	Test(int x=0): data(x) {}

	void *operator new(size_t size){
		cout<<"Test::operator new size: "<<size<<endl;
		void *ptr = malloc(size);
		if (ptr == NULL)
			throw bad_alloc();
		return ptr;
	}

	void operator delete(void *ptr){
		cout<<"Test::operator delete "<<endl;
		free(ptr);
	}
};

int main(){
	Test *ptr = new Test(100);
	delete ptr;
}
