// Purpose: Custom wc command implementation in C++
// Date: 05/01/2026
// Author: Vinayak Majhi
// ID: 58618
// Batch ID: 25SUB4505 - DATACOM+5G

#include <cctype>
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
  int lines = 0, words = 0, chars = 0;
  bool inWord = false;

  while (fin.get(ch)) {
    chars++;

    if (ch == '\n')
      lines++;

    if (isspace(ch))
      inWord = false;
    else if (!inWord) {
      inWord = true;
      words++;
    }
  }

  fin.close();

  cout << lines << " " << words << " " << chars << " " << argv[1] << endl;
  return 0;
}
