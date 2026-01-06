#include <iostream>
using namespace std;

//dynamic array

int main(){ 
	int size;
	int *arr;
	cout<<"Enter the size: ";
	cin>>size;
	arr = new int [size];//allocating the memory

	for (int cnt=0;cnt<size;cnt++)
		arr[cnt] = 1001 + cnt; //filling the array

	for (int cnt=0;cnt<size;cnt++) //printing the data
		cout<<arr[cnt]<<" ";
	cout<<endl;

	delete []arr; //freeing the memory
}
