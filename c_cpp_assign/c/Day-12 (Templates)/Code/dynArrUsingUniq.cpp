// Purpose: Dynamic Array using unique_ptr
// Date: 07/01/2026
// Author: Vinayak Majhi
// ID: 58618
// Batch ID: 25SUB4505 - DATACOM+5G

#include <iostream>
#include <memory>

using namespace std;

unique_ptr<int[]> createArray(int size) {
  if (size <= 0)
    throw invalid_argument("Invalid size");

  unique_ptr<int[]> arr(new int[size]);

  for (int i = 0; i < size; i++)
    arr[i] = (i + 1) * 10;

  return arr;
}

int main() {
  try {
    int size;
    cout << "Enter size: ";
    cin >> size;

    unique_ptr<int[]> arr = createArray(size);

    cout << "Array elements: ";
    for (int i = 0; i < size; i++)
      cout << arr[i] << " ";
    cout << endl;
  } catch (exception &e) {
    cout << "Exception: " << e.what() << endl;
  }

  return 0;
}
