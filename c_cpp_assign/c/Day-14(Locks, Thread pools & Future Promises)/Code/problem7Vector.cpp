/*
 * using vector<int> from STL
 *
 */

#include <iostream>
#include <thread>
#include <vector>
using namespace std;

// Thread function
void fillArray(vector<int> &arr, int startIndex, int count)
{
    for (int i = startIndex; i < startIndex + count; i++) {
        arr[i] = 100 + i;
    }
}

int main()
{
    // vector (shared across threads)
    vector<int> vArr(100);

    const int numThreads = 5;
    const int elementsPerThread = 20;

    vector<thread> threads;

    // Create threads
    for (int i = 0; i < numThreads; i++) {
        threads.emplace_back(
            fillArray,
            ref(vArr),                      // pointer to heap array
            i * elementsPerThread,    // start index
            elementsPerThread        // number of elements
        );
    }

    // Join threads
    for (auto& t : threads) {
        t.join();
    }

    // Print array contents
	cout<<"Array: ";
    for (int i : vArr) {
        cout << i << " ";
    }
	cout<<endl;

    return 0;
}


