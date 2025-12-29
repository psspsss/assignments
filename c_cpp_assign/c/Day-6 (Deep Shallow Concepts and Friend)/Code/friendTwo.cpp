#include <iostream>
using namespace std;

class Box {
  int length;

public:
  Box(int len = 10) : length(len) {}

  friend void showLength(Box &b);
};

void showLength(Box &b) { cout << "Length : " << b.length << endl; }

int main() {
  Box gift;
  showLength(gift);
}
