#include <memory>
#include <iostream>
using namespace std;

int main(){
	unique_ptr<int> ptrOne(new int(100));

	unique_ptr<int> ptrTwo = move(ptrOne); //moving of resource done here 
}
