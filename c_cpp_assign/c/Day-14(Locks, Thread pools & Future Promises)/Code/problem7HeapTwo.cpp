/*
 * using heap based array (Low level)
 *
 *
 */

#include <iostream>
#include <thread>
#include <vector>
using namespace std;

// Thread function
void fillArray(int* arr, int startIndex, int count)
{
    for (int i = startIndex; i < startIndex + count; i++) {
        arr[i] = 100 + i;
    }
}

int main()
{
    
    auto arr = make_unique<int[]>(100); //unique_ptr

    const int numThreads = 5;
    const int elementsPerThread = 20;

    vector<thread> threads;

    // Create threads
    for (int i = 0; i < numThreads; i++) {
        threads.emplace_back(
            fillArray,
            arr.get(),  //using get() pointer to heap array
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
    for (int i = 0; i < 100; i++) {
        cout << arr[i] << " ";
    }
	cout<<endl;

    return 0;
}


