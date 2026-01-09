/*
 * using heap based array (Low level)
 *
 *
 */

#include <iostream>
#include <thread>
#include <vector>

// Thread function
void fillArray(int* arr, int startIndex, int count)
{
    for (int i = startIndex; i < startIndex + count; i++) {
        arr[i] = 100 + i;
    }
}

int main()
{
    //Heap array (shared across threads)
    int *arr = new  int[100];

    const int numThreads = 5;
    const int elementsPerThread = 20;

    std::vector<std::thread> threads;

    // Create threads
    for (int i = 0; i < numThreads; i++) {
        threads.emplace_back(
            fillArray,
            arr,                      // pointer to heap array
            i * elementsPerThread,    // start index
            elementsPerThread        // number of elements
        );
    }

    // Join threads
    for (auto& t : threads) {
        t.join();
    }

    // Print array contents
	std::cout<<"Array: ";
    for (int i = 0; i < 100; i++) {
        std::cout << arr[i] << " ";
    }
	std::cout<<std::endl;

	delete []arr;

    return 0;
}


