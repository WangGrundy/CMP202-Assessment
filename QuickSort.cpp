//#include <thread>
//#include <vector>
//#include <iostream>
//
//void SwapNumbers(int& x, int& y) {
//	int temp = x;
//	x = y;
//	y = temp;
//}
//
//int partition(std::vector<int>& a, int begin, int end)
//{
//	int pivot = a[end]; //pivot is the last element in the array's value
//	int i = begin - 1; // 1 less than subarray, so we can put pivot eventually in correct place
//
//	for (int j = begin; j < end; j++) { //go through entire list left to right
//		if (a[j] < pivot) { //find the position of the element that has a lower number than pivot
//			i++; //I moves along and swaps with j // I is like a chaser, J is the target. I and J swap, eventually this means both sides are averaged with larger and smaller than pivot
//
//			SwapNumbers(a[i], a[j]);
//		}
//	}
//
//	i++; //position of the pivot point in element array in correct position
//	SwapNumbers(a[end], a[i]); //swaps the last element (pivot) into correct location which is i + 1
//
//	return i; //returns position of pivot 
//	//ALL SMALLER SHOULD BE ON THE LEFT OF PIVOT, ALL LARGE IS NOW ON RIGHT OF PIVOT
//}
//
//void QuickSort(std::vector<int>& a, int begin, int end)
//{
//	if (begin < end) {
//		int pivot = partition(a, begin, end);
//
//		QuickSort(a, begin, pivot - 1); //sort smaller (BEGINNING TO PIVOT - 1) // X =========== P - - - - - - - X
//		QuickSort(a, pivot + 1, end); //sort  (PIVOT + 1 TO END)				// X - - - - - - P ============== X
//	}
//}
//
//void QuickSort_P(std::vector<int>& a, int begin, int end)
//{
//
//	if (begin < end) {
//
//		int pivot = partition(a, begin, end);
//
//		std::thread threadA(&QuickSort_P, a, begin, pivot - 1); //sort smaller (BEGINNING TO PIVOT - 1) // X =========== P - - - - - - - X
//		std::thread threadB(&QuickSort, a, pivot + 1, end); //sort  (PIVOT + 1 TO END)				// X - - - - - - P ============== X
//
//		threadA.join();
//		threadB.join();
//	}
//}
//
//// Takes an array and it's size (as int), fills the array with random values from 1 to size*3
//void fillArray(std::vector<int>& a, int size)
//{
//	for (int i = 0; i < size; i++)
//	{
//		a.push_back(rand() % (size * 3));
//	}
//}
//
//// Utility function: O(n) sorted check with early exit.
//bool isSorted(std::vector<int>& a, int size)
//{
//	for (int i = 1; i < size; i++)
//	{
//		if (a[i] < a[i - 1]) return false;
//	}
//	return true;
//}
//
//int main() {
//	//quick sort parallel
//	int size = 10;
//	std::vector <int> a;
//	fillArray(a, size);
//
//	for (int i = 0; i < 10; i++) {
//		std::cout << "BEFORE: " << a[i] << std::endl;
//	}
//
//	//QuickSort(a, 0, size - 1);
//	QuickSort_P(a, 0, size - 1);
//
//	if (isSorted(a, size))
//	{
//		std::cout << "Quicksort sorts correctly:\n";
//	}
//	else {
//		std::cout << "FAIL\n";
//	}
//
//	for (int i = 0; i < 10; i++) {
//		std::cout << "AFTER: " << a[i] << std::endl;
//	}
//}