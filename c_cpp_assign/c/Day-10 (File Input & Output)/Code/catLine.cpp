// Purpose: Custom cat command with line numbers (-n option)
// Date: 05/01/2026
// Author: Vinayak Majhi
// ID: 58618
// Batch ID: 25SUB4505 - DATACOM+5G

#include <cstring>
#include <fstream>
#include <iostream>

using namespace std;

int main(int argc, char *argv[]) {
  if (argc != 3 || strcmp(argv[1], "-n") != 0) {
    cout << "Usage: " << argv[0] << " -n <filename>" << endl;
    return 1;
  }

  ifstream fin(argv[2]);
  if (!fin) {
    cout << "Error opening file" << endl;
    return 1;
  }

  char ch;
  int line = 1;
  cout << line << " ";

  while (fin.get(ch)) {
    cout << ch;
    if (ch == '\n') {
      line++;
      cout << line << " ";
    }
  }

  fin.close();
  return 0;
}
