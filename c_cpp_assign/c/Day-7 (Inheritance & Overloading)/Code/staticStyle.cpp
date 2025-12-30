#include <cstring>
#include <iostream>
using namespace std;

int main() {
  char str[] = "One way of using a class is by creating objects and using the "
               "member through that object";
  char *ptr = strtok(str, " "); // FIRST time giving address
  while (ptr != NULL) {
    cout << ptr << endl;
    ptr = strtok(NULL, " "); // consecutive no address given
  }
}
