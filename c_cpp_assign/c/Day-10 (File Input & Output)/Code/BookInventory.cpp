// Purpose: Book Inventory System using Binary File
// Date: 05/01/2026
// Author: Vinayak Majhi
// ID: 58618
// Batch ID: 25SUB4505 - DATACOM+5G

#include <cstring>
#include <fstream>
#include <iostream>

using namespace std;

class Book {
  int bookId;
  char title[40];
  float price;

public:
  void input() {
    cout << "Enter Book ID: ";
    cin >> bookId;
    cin.ignore();
    cout << "Enter Title: ";
    cin.getline(title, 40);
    cout << "Enter Price: ";
    cin >> price;
  }

  void display() const {
    cout << bookId << "\t" << title << "\t" << price << endl;
  }
};

int main() {
  Book b;
  ofstream fout("books.dat", ios::binary | ios::app);
  b.input();
  fout.write((char *)&b, sizeof(b));
  fout.close();

  ifstream fin("books.dat", ios::binary);
  cout << "ID\tTitle\tPrice\n";
  while (fin.read((char *)&b, sizeof(b))) {
    b.display();
  }
  fin.close();

  return 0;
}
