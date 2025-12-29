#include <cstdlib>
#include <ctime>
#include <iostream>

using namespace std;

int main() {
  int n;
  cout << "Enter size: ";
  cin >> n;

  int *arr = new int[n];
  srand(time(0));

  for (int i = 0; i < n; i++) {
    arr[i] = rand() % 100;
    cout << arr[i] << " ";
  }

  for (int i = 0; i < n - 1; i++) {
    for (int j = 0; j < n - i - 1; j++) {
      if (arr[j] > arr[j + 1]) {
        int temp = arr[j];
        arr[j] = arr[j + 1];
        arr[j + 1] = temp;
      }
    }
  }

  cout << "\nSorted array:\n";
  for (int i = 0; i < n; i++) {
    cout << arr[i] << " ";
  }

  delete[] arr;
  return 0;
}
