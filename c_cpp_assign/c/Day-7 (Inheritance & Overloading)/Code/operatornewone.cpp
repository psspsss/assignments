#include <cstdlib>
#include <iostream>
using namespace std;

void *operator new(size_t size) {
  cout << "Global operator new with size: " << size << endl;
  void *ptr = malloc(size);
  if (ptr == NULL) {
    throw bad_alloc();
  }
  return ptr;
}

void operator delete(void *ptr) {
  cout << "Global operator delete" << endl;
  free(ptr);
}

struct Test {
  int data;
  Test(int x = 0) : data(x) { cout << "Test()" << endl; }
  ~Test() { cout << "~Test()" << endl; }
};

int main() {
  Test *ptr = new Test();
  delete ptr;
}
