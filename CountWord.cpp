#define _SILENCE_AMP_DEPRECATION_WARNINGS

#include <iostream>
#include <amp.h>
#include <thread>
#include <mutex>

#include <chrono>
#include <iomanip>
#include <time.h>
#include <array>
#include <numeric>
#include <algorithm>
#include <condition_variable>
#include <semaphore>
#include <barrier>


using namespace concurrency;
using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::chrono::nanoseconds;
typedef std::chrono::steady_clock the_serial_clock;
typedef std::chrono::steady_clock the_amp_clock;

bool timerStarted = false;
the_amp_clock::time_point end1;

std::atomic_int elementsDoneAtomic = -1;
std::atomic_int countAtomic = 0;

std::condition_variable cv;
bool lockTimer = false;
std::mutex mutexForTimer;

std::atomic_bool atomicBool = false;
std::barrier allThreads(5);

std::string GenerateRandomString(int sizeOfWord) {
	std::string temp;

	char allChars[] =
		"0123456789"
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz";
	
	temp.reserve(sizeOfWord);

	for (int i = 0; i < sizeOfWord; ++i) {
		temp += allChars[rand() % (sizeof(allChars) - 1)];
	}

	return temp;
}

void GenerateVectorOfWords(std::string word, std::vector<std::string>& vector, int sizeOfVector, int sizeOfRandomWords) {
	
	for (int i = 0; i < sizeOfVector; i++) {
		
		int randomNum = rand() % 10;

		if (randomNum == 1) {
			vector.push_back(word);
		}
		else {
			vector.push_back(GenerateRandomString(sizeOfRandomWords)); // emplace_back forwards the arguments to a constructor (size, char)
		}
	}
}

void WordCount(std::string word, const std::vector<std::string>& vector, int& count) {
	
	for (int i = 0; i < vector.size(); i++) {
		if (vector[i] == word) {
			count++;
		}
	}

	/*for (std::string x : vector) {
		if (x == word) {
			count++;
		}
	}*/
}

// doesn't work effectively
//void WordCount_P_Atomic(std::string word, const std::vector<std::string>& vector, int threadsNum, int& count) { 
//
//	auto claim_index = []() -> int { //each thread can "claim" some spot in the vector using the value returned by fetch_add
//		return elementsDoneAtomic.fetch_add(1, std::memory_order_seq_cst); //add 1 to element checked counter  //single total order exists in which all threads observe all modifications in the same order with {memory_order_seq_cst}
//	};  // returns the value immediately preceding the effects of this function in the modification order of *this, AKA elementsDoneAtomic + 1
//
//	int claimed_index;
//
//	//if (!timerStarted) {
//	//	timerStarted = true;
//	//	start1 = the_amp_clock::now(); //start timer
//	//	
//	//}
//
//	//start1 = the_amp_clock::now(); //start timer
//
//	while ((claimed_index = claim_index()) < vector.size()) {
//		std::cout << claimed_index << std::endl;
//
//		//if a word in the vector is equal to the word we are looking for,
//		if (vector[claimed_index] == word) {
//			countAtomic++; //add 1 to the word count
//		}
//	}
//
//	end1 = the_amp_clock::now(); //end timer
//	count = countAtomic; //set the count with the atomic count
//}

void WordCount_P_Atomic_2(std::string word, const std::vector<std::string>& vector, int threadTotal, int& count, int threadID) {

	int threadNumber = threadID + 1;
	int threadCountTemp = threadNumber;

	//std::unique_lock<std::mutex> uniquelock1(mutexForTimer);

	//if (lockTimer == false) {
	//	std::cout << "waiting" << std::endl;
	//	cv.wait(uniquelock1);
	//	//std::cout << "GO" << std::endl;
	//}
	
	allThreads.arrive_and_wait();

	if (threadNumber == 1) { //account for the 0th element
		if (vector[0] == word) {
			countAtomic++;
		}
	}

	while (threadCountTemp < vector.size()) {

		if (vector[threadCountTemp] == word) {
			countAtomic++;
		}

		threadCountTemp += threadTotal;
	}

	end1 = the_amp_clock::now(); //end timer
	count = countAtomic; //set the count with the atomic count
}

long long TestNonParallel(int& count, std::string word, int sizeOfVector, const std::vector<std::string>& vector) {
	
	the_amp_clock::time_point start = the_amp_clock::now(); ///////////////////////////////////////
	WordCount(word, vector, count);
	the_amp_clock::time_point end = the_amp_clock::now();
	return duration_cast<nanoseconds>(end - start).count(); ///////////////////////////////////////
}

long long TestParallel(int& count, std::string word, int sizeOfVector, std::vector<std::string>& vector, int threadsNum) {
	
	//test
	atomicBool = false;

	std::mutex count_mutex;
	std::mutex elementsDone_mutex;
	int elementsDone = 0;
	std::vector<std::thread> threadVector;
	lockTimer = false;

	{
		std::lock_guard<std::mutex> lg(mutexForTimer); //lock guard
		for (int i = 0; i < threadsNum; i++) { //do work
			threadVector.push_back(std::thread(&WordCount_P_Atomic_2, word, vector, threadsNum, std::ref(count), i));
		}
		lockTimer = true; //set bool to true
		cv.notify_all(); //notify all threads that thread creation has been complete
	}
	//start timing
	the_amp_clock::time_point start1 = the_amp_clock::now(); //start timer
	
	for (int i = 0; i < threadsNum; i++) {
		threadVector[i].join();
	}

	return duration_cast<nanoseconds>(end1 - start1).count(); ///////////////////////////////////////
}

int main() {
	srand(time(0));

	std::string word = "hello";
	int count = 0;
	int count2 = 0;
	int cores = std::thread::hardware_concurrency();
	int threadsNum = cores - 1;
	std::cout << "number of cores: " << cores << std::endl;
	int sizeOfVector = 100000;
	int sizeOfRandomWords = 5;
	std::vector<std::string> vector;

	GenerateVectorOfWords(word, vector, sizeOfVector, sizeOfRandomWords);
	/*for (std::string word : vector) {
		std::cout << word << std::endl;
	}*/

	int testRepetitions = 10;
	std::vector<long long> parallelTestTimes,  nonParallelTestTimes;

	for (int i = 0; i < testRepetitions; i++) {
		//std::cout << "starting test[ " << i << " ] " << std::endl;
		nonParallelTestTimes.push_back(TestNonParallel(count, word, sizeOfVector, vector));
		parallelTestTimes.push_back(TestParallel(count2, word, sizeOfVector, vector, threadsNum));
	}

	std::sort(nonParallelTestTimes.begin(), nonParallelTestTimes.end());
	std::sort(parallelTestTimes.begin(), parallelTestTimes.end());

	int medianPlacement = (testRepetitions / 2) - 1;

	//for (long long num : parallelTestTimes) {
	//	std::cout << num << std::endl;
	//}

	std::cout << "SERIAL: COUNT IS : " << count << " AND TIME TAKEN: "  << nonParallelTestTimes[medianPlacement] << "ms" << std::endl;
	std::cout << "PARALLEL: COUNT IS : " << count2 << " AND TIME TAKEN: "<< parallelTestTimes[medianPlacement] << "ms" << std::endl;
}