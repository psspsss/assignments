#include <iostream>

using namespace std;

namespace Mine {
int var = 100;
void fun();
} // namespace Mine

namespace Yours {
int var = 300;
void fun();
} // namespace Yours

namespace Ours {
int var = 900;
void fun();
} // namespace Ours
//
//
int var = 10;

void fun();

int main() {
  cout << "global var: " << ::var << endl;
  cout << "Mine::var = " << Mine::var << endl;
  cout << "Yours::var = " << Yours::var << endl;
  cout << "Ours::var = " << Ours::var << endl;

  fun();
  Mine::fun();
  Yours::fun();
  Ours::fun();
}
void fun() { cout << "void fun()...\n"; }
void Mine::fun() { cout << "void Mine::fun()...\n"; }
void Yours::fun() { cout << "void Yours::fun()...\n"; }
void Ours::fun() { cout << "void Ours::fun()...\n"; }
