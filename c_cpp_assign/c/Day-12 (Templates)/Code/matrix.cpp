// Purpose: Matrix Operations using Class Templates
// Date: 07/01/2026
// Author: Vinayak Majhi
// ID: 58618
// Batch ID: 25SUB4505 - DATACOM+5G

#include <iostream>

using namespace std;

template <class T> class Matrix {
  int rows, cols;
  T mat[10][10];

public:
  Matrix(int r, int c) : rows(r), cols(c) {}

  void input() {
    for (int i = 0; i < rows; i++)
      for (int j = 0; j < cols; j++)
        cin >> mat[i][j];
  }

  Matrix add(const Matrix &m) const {
    Matrix result(rows, cols);
    for (int i = 0; i < rows; i++)
      for (int j = 0; j < cols; j++)
        result.mat[i][j] = mat[i][j] + m.mat[i][j];
    return result;
  }

  void display() const {
    for (int i = 0; i < rows; i++) {
      for (int j = 0; j < cols; j++)
        cout << mat[i][j] << " ";
      cout << endl;
    }
  }
};

int main() {
  Matrix<int> m1(2, 2), m2(2, 2);
  cout << "Enter Matrix 1:\n";
  m1.input();
  cout << "Enter Matrix 2:\n";
  m2.input();

  Matrix<int> m3 = m1.add(m2);
  cout << "Result Matrix:\n";
  m3.display();

  return 0;
}
