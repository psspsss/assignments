#include <iostream>

using namespace std;

class Test {

  // private member by default

  void funOne() { cout << "Test::funOne()--> Private" << endl; }

protected:
  void funTwo() { cout << "Test::funTwo() --> Protected" << endl; }

public:
  void funThree() { cout << "Test::funThree() --> Public" << endl; }
};

class Derived : public Test {

public:
  void funFour() {

    // funOne(); //private member

    funTwo(); // protected member inherited here

    funThree(); // public member inherited here
  }
};

int main() {

  Derived dObj;

  dObj.funThree();

  dObj.funFour();
}
