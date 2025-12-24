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

  int max = arr[0], min = arr[0];

  for (int i = 1; i < n; i++) {
    if (arr[i] > max)
      max = arr[i];
    if (arr[i] < min)
      min = arr[i];
  }

  cout << "\nLargest = " << max;
  cout << "\nSmallest = " << min;

  delete[] arr;
  return 0;
}
