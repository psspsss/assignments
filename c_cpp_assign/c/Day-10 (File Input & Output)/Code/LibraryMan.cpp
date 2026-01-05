// Purpose: Library Management System using Binary File
// Date: 05/01/2026
// Author: Vinayak Majhi
// ID: 58618
// Batch ID: 25SUB4505 - DATACOM+5G

#include <cstring>
#include <fstream>
#include <iostream>

using namespace std;

class Library {
  int bookId;
  char bookName[40];
  char author[30];
  bool available;

public:
  void input() {
    cout << "Enter Book ID: ";
    cin >> bookId;
    cin.ignore();
    cout << "Enter Book Name: ";
    cin.getline(bookName, 40);
    cout << "Enter Author: ";
    cin.getline(author, 30);
    available = true;
  }

  void display() const {
    cout << bookId << "\t" << bookName << "\t" << author << "\t"
         << (available ? "Available" : "Issued") << endl;
  }

  int getId() const { return bookId; }

  void issue() { available = false; }
  void ret() { available = true; }
};

void addBook() {
  Library l;
  ofstream fout("library.dat", ios::binary | ios::app);
  l.input();
  fout.write((char *)&l, sizeof(l));
  fout.close();
}

void issueBook(int id) {
  Library l;
  fstream file("library.dat", ios::binary | ios::in | ios::out);

  while (file.read((char *)&l, sizeof(l))) {
    if (l.getId() == id) {
      l.issue();
      file.seekp(-sizeof(l), ios::cur);
      file.write((char *)&l, sizeof(l));
      break;
    }
  }
  file.close();
}

void returnBook(int id) {
  Library l;
  fstream file("library.dat", ios::binary | ios::in | ios::out);

  while (file.read((char *)&l, sizeof(l))) {
    if (l.getId() == id) {
      l.ret();
      file.seekp(-sizeof(l), ios::cur);
      file.write((char *)&l, sizeof(l));
      break;
    }
  }
  file.close();
}

void deleteBook(int id) {
  Library l;
  ifstream fin("library.dat", ios::binary);
  ofstream fout("temp.dat", ios::binary);

  while (fin.read((char *)&l, sizeof(l))) {
    if (l.getId() != id)
      fout.write((char *)&l, sizeof(l));
  }

  fin.close();
  fout.close();
  remove("library.dat");
  rename("temp.dat", "library.dat");
}

int main() {
  addBook();
  issueBook(1);
  returnBook(1);
  deleteBook(1);
  return 0;
}
