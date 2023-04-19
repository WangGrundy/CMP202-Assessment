//#include <iostream>
//#include <amp.h>
//#include <thread>
//
//#include <chrono>
//#include <iomanip>
//#include <time.h>
//#include <array>
//#include <numeric>
//
//using namespace concurrency;
//using std::chrono::duration_cast;
//using std::chrono::milliseconds;
//typedef std::chrono::steady_clock the_serial_clock;
//typedef std::chrono::steady_clock the_amp_clock;
//
//const int TS = 1024;
//
//void GenerateRandomNumbers(std::vector<int>& storage, int numberOfElements) {
//	
//	int randomNum = 0;
//	
//	for (int i = 0; i < numberOfElements; i++) {
//		randomNum = rand();
//		storage.push_back(randomNum);
//	}
//}
//
//void GenerateSameNumber(std::vector<int>& storage, int numberOfElements, int numberToAttach) {
//
//	for (int i = 0; i < numberOfElements; i++) {
//		storage.push_back(numberToAttach);
//	}
//}
//
//long long AddTwoVectors_Threaded(std::vector<int>& storage1, std::vector<int>& storage2, std::vector<int>& resultStorage, int size){
//	array_view<int, 1> arr1(size, storage1);
//	array_view<int, 1> arr2(size, storage2);
//	extent<1> ex(TS);
//	array_view<int, 1> arrResult(ex, resultStorage);
//
//	arrResult.discard_data();
//
//	the_amp_clock::time_point start = the_amp_clock::now();
//
//	parallel_for_each(arrResult.extent, [=](index<1> index) restrict(amp) {
//
//		arrResult[index] = arr1[index] + arr2[index];
//	});
//
//	arrResult.synchronize();  //gpu to cpu here
//	the_amp_clock::time_point end = the_amp_clock::now();
//	auto time_taken = duration_cast<milliseconds>(end - start).count();
//
//	return time_taken;
//}
//
//long long AddTwoVectors_TileThreaded(std::vector<int>& storage1, std::vector<int>& storage2, std::vector<int>& resultStorage, int size) {
//	array_view<int, 1> arr1(size, storage1);
//	array_view<int, 1> arr2(size, storage2);
//	extent<1> ex(TS);
//	array_view<int, 1> arrResult(ex, resultStorage);
//
//	arrResult.discard_data();
//
//	the_amp_clock::time_point start = the_amp_clock::now(); //----------------------------------------------
//
//	parallel_for_each(arrResult.extent.tile<TS>(), [=](tiled_index<TS> index) restrict(amp) {
//
//		arrResult[index.global] = arr1[index] + arr2[index];
//		});
//
//	arrResult.synchronize(); //gpu to cpu here
//	the_amp_clock::time_point end = the_amp_clock::now(); //----------------------------------------------
//	auto time_taken = duration_cast<milliseconds>(end - start).count();
//
//	return time_taken;
//}
//
//long long AddTwoVectors_TileStaticThreaded(std::vector<int>& storage1, std::vector<int>& storage2, std::vector<int>& resultStorage, int size) {
//	array_view<int, 1> arr1(size, storage1);
//	array_view<int, 1> arr2(size, storage2);
//	extent<1> ex(TS);
//	array_view<int, 1> arrResult(ex, resultStorage);
//
//	arrResult.discard_data();
//
//	the_amp_clock::time_point start = the_amp_clock::now(); //----------------------------------------------
//
//	parallel_for_each(arrResult.extent.tile<TS>(), [=](tiled_index<TS> index) restrict(amp) {
//		
//		tile_static double arr1_ts[TS];
//		tile_static double arr2_ts[TS];
//
//		arr1_ts[index.local[0]] = arr1[index.global[0]]; //copy arrays from global to local mem
//		arr2_ts[index.local[0]] = arr2[index.global[0]];
//
//		index.barrier.wait(); //idk if this is needed
//
//		arrResult[index.global[0]] = arr1_ts[index.local[0]] + arr2_ts[index.local[0]]; //do sum and put back into global
//		});
//
//	arrResult.synchronize();
//	the_amp_clock::time_point end = the_amp_clock::now(); //----------------------------------------------
//	auto time_taken = duration_cast<milliseconds>(end - start).count();
//
//	return time_taken;
//}
//
//void testSpeed() { //old main
//	time_t current_time = time(NULL);
//	srand(current_time);
//	int numberOfElements = 50000000;
//
//	std::vector<int> storage;
//	std::vector<int> storage2;
//	std::vector<int> storageResult(numberOfElements);
//
//	GenerateRandomNumbers(storage, numberOfElements);
//	GenerateSameNumber(storage2, numberOfElements, 5);
//
//	for (int i = 0; i < 10; i++) {
//		std::cout << "BEFORE: " << storage[i] << std::endl;
//	}
//
//	AddTwoVectors_Threaded(storage, storage2, storageResult, numberOfElements);
//	long long timeTaken = AddTwoVectors_Threaded(storage, storage2, storageResult, numberOfElements);
//	long long timeTaken2 = AddTwoVectors_TileThreaded(storage, storage2, storageResult, numberOfElements);
//	long long timeTaken3 = AddTwoVectors_TileStaticThreaded(storage, storage2, storageResult, numberOfElements);
//
//	std::cout << std::endl << std::endl;
//
//	for (int i = 0; i < 10; i++) {
//		std::cout << "AFTER: " << storageResult[i] << std::endl;
//	}
//
//	std::cout << "threaded: " << timeTaken << "ms" << std::endl;
//	std::cout << "tileThreaded: " << timeTaken2 << "ms" << std::endl;
//	std::cout << "tileStaticThreaded: " << timeTaken3 << "ms" << std::endl;
//}
//
//#define TILESIZE 2
//
//float Reduction(std::vector<int>& vec, int numberOfElements) {
//
//	array<float, 1> arr_1(numberOfElements, vec.begin());
//	array<float, 1> arr_2((numberOfElements / TILESIZE) ? (numberOfElements / TILESIZE) : 1);
//	float total_time = 0;
//	// array_views may be swapped after each iteration.
//	array_view<float, 1> av_src(arr_1);
//	array_view<float, 1> av_dst(arr_2);
//	av_dst.discard_data();
//
//	while ((numberOfElements % TILESIZE) == 0)
//	{
//		parallel_for_each(extent<1>(numberOfElements).tile<TILESIZE>(), [=](tiled_index<TILESIZE> tidx) restrict(amp)
//			{
//				// Use tile_static as a scratchpad memory.
//				tile_static float tile_data[TILESIZE];
//
//				unsigned int local_idx = tidx.local[0];
//				tile_data[local_idx] = av_src[tidx.global];
//				tidx.barrier.wait();
//
//				for (int s = 1; s < TILESIZE; s *= 2)
//				{
//					if (local_idx % (2 * s) == 0)
//					{
//						tile_data[tidx.local[0]] += tile_data[tidx.local[0] + s];
//					}
//					tidx.barrier.wait();
//				}
//
//				// Store the tile result in the global memory.
//				if (local_idx == 0)
//				{
//					av_dst[tidx.tile] = tile_data[0];
//				}
//			});//end of parallel_for_each
//
//		// Update the sequence length, swap source with destination.
//		numberOfElements /= TILESIZE;
//		std::swap(av_src, av_dst);
//		av_dst.discard_data();
//	}
//	// Perform any remaining reduction on the CPU.
//	std::vector<float> result(numberOfElements);
//	for (int i = 0; i < numberOfElements; i++) {
//		result.push_back(av_src[i]);
//	}
//
//	return std::accumulate(result.begin(), result.end(), 0.f);
//}
//
////void Hello(int x) {
////	std::cout << "HELLO " << x;
////}
//
//int main() {
//	int cores = std::thread::hardware_concurrency();
//	int numberOfThreads = cores - 1;
//	std::cout << "number of cores: " << cores << std::endl;
//
//	/*int numberOfElements = 10000;
//
//	std::vector<int> storage(numberOfElements, 1);
//
//	for (int i = 0; i < 10; i++) {
//		std::cout << "BEFORE: " << storage[i] << std::endl;
//	}
//
//	float x = Reduction(storage, numberOfElements);
//
//	for (int i = 0; i < 10; i++) {
//		std::cout << "AFTER: " << x << std::endl;
//	}*/
//}