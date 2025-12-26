#include <iostream>
using namespace std;

class Num {
  int num;

public:
  Num(int n = 10) { num = n; }

  void dispNum() { cout << "Number: " << num << endl; }

  bool isEven() { return (num % 2 == 0); }

  bool isPrime() {
    if (num <= 1)
      return false;

    for (int i = 2; i * i <= num; i++) {
      if (num % i == 0)
        return false;
    }
    return true;
  }
};

int main() {
  Num n1; // default value 10
  n1.dispNum();
  cout << "Even: " << n1.isEven() << endl;
  cout << "Prime: " << n1.isPrime() << endl;

  Num n2(17); // custom value
  n2.dispNum();
  cout << "Even: " << n2.isEven() << endl;
  cout << "Prime: " << n2.isPrime() << endl;

  return 0;
}
