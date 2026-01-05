// Purpose: Custom cat command implementation in C++
// Date: 05/01/2026
// Author: Vinayak Majhi
// ID: 58618
// Batch ID: 25SUB4505 - DATACOM+5G

#include <fstream>
#include <iostream>

using namespace std;

int main(int argc, char *argv[]) {
  if (argc != 2) {
    cout << "Usage: " << argv[0] << " <filename>" << endl;
    return 1;
  }

  ifstream fin(argv[1]);
  if (!fin) {
    cout << "Error opening file" << endl;
    return 1;
  }

  char ch;
  while (fin.get(ch)) {
    cout << ch;
  }

  fin.close();
  return 0;
}
