/*
 *	Filling an array of 100 elements with 5 threads
 *
 *		Using global array variable
 *
 */

#include <iostream>
#include <thread>
#include <vector>

// Global array of 100 elements
int arr[100];

// Thread function
void fillArray(int startIndex, int count)
{
    for (int i = startIndex; i < startIndex + count; i++) {
        arr[i] = 100 + i;   // Fill with thread ID for visibility
    }
}

int main()
{
    const int numThreads = 5;
    const int elementsPerThread = 20;

    std::vector<std::thread> threads;

    // Create 5 threads
    for (int i = 0; i < numThreads; i++) {
        threads.emplace_back(
            fillArray,
            i * elementsPerThread,   // start index
            elementsPerThread
        );
    }

    // Wait for all threads to complete
    for (auto& t : threads) {
        t.join();
    }

	std::cout<<"Array: ";
    // Verify the result
    for (int i = 0; i < 100; i++) {
        std::cout << arr[i] << " ";
    }
	std::cout<<std::endl;
    return 0;
}

