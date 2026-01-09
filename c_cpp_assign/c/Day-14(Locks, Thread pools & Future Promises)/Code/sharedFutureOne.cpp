#include <iostream>
#include <thread>
#include <future>
using namespace std;

void calculateSum(promise<int> prom)
{
    int sum = 0;
    for (int i = 1; i <= 10; i++) {
        sum += i;
    }
    cout << "Sum calculated\n";
    prom.set_value(sum);
}

void readResult(shared_future<int> sf, int id)
{
    cout << "Reader " << id
         << " got sum = " << sf.get() << endl;
}

int main()
{
    promise<int> p;
    shared_future<int> sf = p.get_future().share();
    thread producer(calculateSum, move(p));

    thread t1(readResult, sf, 1);
    thread t2(readResult, sf, 2);
    thread t3(readResult, sf, 3);

    producer.join();
    t1.join();
    t2.join();
    t3.join();
}

