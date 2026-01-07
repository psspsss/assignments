// Purpose: Generic Stack using Templates
// Date: 07/01/2026
// Author: Vinayak Majhi
// ID: 58618
// Batch ID: 25SUB4505 - DATACOM+5G

#include <iostream>

using namespace std;

template <class T> class MyStack {
  T *arr;
  int topIndex;
  int capacity;

public:
  MyStack(int cap = 10) {
    capacity = cap;
    arr = new T[capacity];
    topIndex = -1;
  }

  void push(T value) {
    if (topIndex == capacity - 1) {
      cout << "Stack Overflow\n";
      return;
    }
    arr[++topIndex] = value;
  }

  void pop() {
    if (isEmpty()) {
      cout << "Stack Underflow\n";
      return;
    }
    topIndex--;
  }

  T top() const {
    if (isEmpty())
      throw runtime_error("Stack is empty");
    return arr[topIndex];
  }

  bool isEmpty() const { return topIndex == -1; }

  ~MyStack() { delete[] arr; }
};

int main() {
  MyStack<int> s(5);
  s.push(10);
  s.push(20);

  cout << "Top: " << s.top() << endl;
  s.pop();
  cout << "Top after pop: " << s.top() << endl;

  return 0;
}
