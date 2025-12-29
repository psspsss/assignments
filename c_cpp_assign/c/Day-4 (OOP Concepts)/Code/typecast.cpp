#include <iostream>
using namespace std;

int main() {
  int a = 10;
  float b = 3.14f;
  char c = 'X';

  int *ip = &a;
  float *fp = &b;
  char *cp = &c;

  fp = ip;
  cp = ip;
  ip = cp;

  cout << "Addresses:" << endl;
  cout << ip << endl;
  cout << fp << endl;
  cout << cp << endl;

  return 0;
}
