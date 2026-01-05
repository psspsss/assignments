// Purpose: Student Record Management using Binary File
// Date: 05/01/2026
// Author: Vinayak Majhi
// ID: 58618
// Batch ID: 25SUB4505 - DATACOM+5G

#include <cstring>
#include <fstream>
#include <iostream>

using namespace std;

class Student {
  int roll;
  char name[30];
  float marks;

public:
  void input() {
    cout << "Enter Roll No: ";
    cin >> roll;
    cin.ignore();
    cout << "Enter Name: ";
    cin.getline(name, 30);
    cout << "Enter Marks: ";
    cin >> marks;
  }

  void display() const {
    cout << roll << "\t" << name << "\t" << marks << endl;
  }

  int getRoll() const { return roll; }

  void setMarks(float m) { marks = m; }
};

void addStudent() {
  Student s;
  ofstream fout("student.dat", ios::binary | ios::app);
  s.input();
  fout.write((char *)&s, sizeof(s));
  fout.close();
}

void displayAll() {
  Student s;
  ifstream fin("student.dat", ios::binary);
  cout << "Roll\tName\tMarks\n";
  while (fin.read((char *)&s, sizeof(s))) {
    s.display();
  }
  fin.close();
}

void updateMarks(int r) {
  Student s;
  fstream file("student.dat", ios::binary | ios::in | ios::out);

  while (file.read((char *)&s, sizeof(s))) {
    if (s.getRoll() == r) {
      float m;
      cout << "Enter new marks: ";
      cin >> m;
      s.setMarks(m);
      file.seekp(-sizeof(s), ios::cur);
      file.write((char *)&s, sizeof(s));
      break;
    }
  }
  file.close();
}

void deleteStudent(int r) {
  Student s;
  ifstream fin("student.dat", ios::binary);
  ofstream fout("temp.dat", ios::binary);

  while (fin.read((char *)&s, sizeof(s))) {
    if (s.getRoll() != r)
      fout.write((char *)&s, sizeof(s));
  }

  fin.close();
  fout.close();
  remove("student.dat");
  rename("temp.dat", "student.dat");
}

int main() {
  addStudent();
  displayAll();
  updateMarks(1);
  deleteStudent(1);
  return 0;
}
