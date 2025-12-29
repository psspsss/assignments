#include <iostream>
using namespace std;

class Singleton {
private: // data members here
  static Singleton *ptr2Obj;

  Singleton() { cout << "Object created..." << endl; }
  Singleton(int x) { cout << "Object created...with " << x << endl; }
  Singleton(const Singleton &);

public:
  static Singleton &getSingleton() {
    if (ptr2Obj == nullptr)
      ptr2Obj = new Singleton();

    return *ptr2Obj;
  }
  void disp() { cout << "Singleton::disp()" << endl; }
};

Singleton *Singleton::ptr2Obj = nullptr;

int main() {
  // Singleton objOne;//this is an error

  Singleton &refObj = Singleton::getSingleton();
  refObj.disp();
}
