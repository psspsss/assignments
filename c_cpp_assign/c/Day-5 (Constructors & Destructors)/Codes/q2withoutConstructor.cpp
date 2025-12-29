#include <iostream>
using namespace std;

class Num {
  int num;

public:
  void setNum(int n = 10) { num = n; }

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
  Num n1;
  n1.setNum();
  n1.dispNum();
  cout << "Even: " << n1.isEven() << endl;
  cout << "Prime: " << n1.isPrime() << endl;

  Num n2;
  n2.setNum(17);
  n2.dispNum();
  cout << "Even: " << n2.isEven() << endl;
  cout << "Prime: " << n2.isPrime() << endl;

  return 0;
}
