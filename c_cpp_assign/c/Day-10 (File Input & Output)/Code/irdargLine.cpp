#include <iostream>

#include <fstream>

using namespace std;

int main() {

  ifstream fobj("ioThree.cpp");

  string str;

  while (fobj >> str) // word by word

    cout << str << " ";
}

#include <iostream>

#include <fstream>

using namespace std;

int main() {

  ifstream fobj("ioThree.cpp");

  string str;

  while (getline(fobj, str)) // word by word

    cout << str << endl;
}
