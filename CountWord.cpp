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
#include <barrier>

using namespace concurrency;
using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::chrono::nanoseconds;
typedef std::chrono::steady_clock the_serial_clock;
typedef std::chrono::steady_clock the_amp_clock;

bool timerStarted = false;
the_amp_clock::time_point end1;
the_amp_clock::time_point start1;
std::atomic_int countAtomic = 0;
std::condition_variable cv;
std::mutex mutexForTimer;
int cores = 0;
int threadsNum = 0;
std::barrier allThreads(2); //manually change this

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
		
		int randomNum = rand() % 10; //10% chance of our word appearing

		if (randomNum == 1) { //10% chance of our word appearing
			vector.push_back(word);
		}
		else {
			vector.push_back(GenerateRandomString(sizeOfRandomWords)); //90% chance of random string
		}
	}
}

void WordCount(std::string word, const std::vector<std::string>& vector, int& count) {
	
	/*for (int i = 0; i < vector.size(); i++) {
		if (vector[i] == word) {
			count++;
		}
	}*/

	for (std::string x : vector) {
		if (x == word) {
			count++;
		}
	}
}

void WordCount_P_Atomic_2(std::string word, const std::vector<std::string>& vector, int threadTotal, int& count, int threadID) {

	int threadNumber = threadID + 1;
	int threadCountTemp = threadNumber;

	std::unique_lock<std::mutex> uniquelock1(mutexForTimer);

	std::cout << "waiting" << std::endl;
	cv.wait(uniquelock1);
	start1 = the_amp_clock::now(); //start timer
	std::cout << "GO" << std::endl;

	if (threadNumber == 1) { //account for the 0th element if 1st thread
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

void WordCount_P_Atomic_3(std::string word, const std::vector<std::string>& vector, int threadTotal, int& count, int threadID) {

	int threadNumber = threadID + 1;
	int threadCountTemp = threadNumber;

	std::unique_lock<std::mutex> uniquelock1(mutexForTimer);

	the_amp_clock::time_point a = the_amp_clock::now();
	allThreads.arrive_and_wait(); //all threads wait here then start timer.
	the_amp_clock::time_point b = the_amp_clock::now();

	std::cout << "this thread waited: " << duration_cast<milliseconds>(b - a).count() << std::endl;

	if (threadNumber == 1) { //account for the 0th element if 1st thread
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
	return duration_cast<milliseconds>(end - start).count(); ///////////////////////////////////////
}

long long TestParallel(int& count, std::string word, int sizeOfVector, std::vector<std::string>& vector, int threadsNum) {

	std::vector<std::thread> threadVector;

	for (int i = 0; i < threadsNum; i++) { //do work
		threadVector.push_back(std::thread(&WordCount_P_Atomic_2, word, vector, threadsNum, std::ref(count), i));
	}
	std::this_thread::sleep_for(std::chrono::seconds(1));
	cv.notify_all(); //notify all threads that thread creation has been complete

	for (int i = 0; i < threadsNum; i++) {
		threadVector[i].join();
	}

	return duration_cast<milliseconds>(end1 - start1).count(); ///////////////////////////////////////
}

int main() {
	srand(time(0));
	cores = std::thread::hardware_concurrency();
	std::cout << "number of cores: " << cores << std::endl;
	int count = 0;
	int count2 = 0;
	std::vector<std::string> vector;
	std::vector<long long> parallelTestTimes, nonParallelTestTimes;

	//PLEASE CHANGE TO YOUR LIKING ---------------------------------------
	threadsNum = cores - 1; //should be equal to number of cores - 1 because it includes main thread (advisory)
	//threadsNum = 2;
	std::string word = "hello"; //our word that we will add to the randomly generated vector
	int sizeOfVector = 100000; //the size of how large our vector of words will be to search through
	int sizeOfRandomWords = 10; //size of each word inserted into the vector
	int testRepetitions = 10; //number of times this test will be repeated
	//PLEASE CHANGE TO YOUR LIKING ---------------------------------------

	GenerateVectorOfWords(word, vector, sizeOfVector, sizeOfRandomWords); //insert our word randomly into this

	/*for (std::string word : vector) {
		std::cout << word << std::endl;
	}*/

	for (int i = 0; i < testRepetitions; i++) {
		std::cout << "starting test[ " << i << " ] " << std::endl;
		nonParallelTestTimes.push_back(TestNonParallel(count, word, sizeOfVector, vector));
		parallelTestTimes.push_back(TestParallel(count2, word, sizeOfVector, vector, threadsNum));
	}

	std::sort(nonParallelTestTimes.begin(), nonParallelTestTimes.end());
	std::sort(parallelTestTimes.begin(), parallelTestTimes.end());

	int medianPlacement = (testRepetitions / 2) - 1;

	for (long long num : parallelTestTimes) {
		std::cout << num << std::endl;
	}

	std::cout << "SERIAL: COUNT IS : " << count << " AND TIME TAKEN: "  << nonParallelTestTimes[medianPlacement] << "ns" << std::endl;
	std::cout << "PARALLEL: COUNT IS : " << count2 << " AND TIME TAKEN: "<< parallelTestTimes[medianPlacement] << "ns" << std::endl;
	std::cout << "parallel is faster by " << nonParallelTestTimes[medianPlacement] - parallelTestTimes[medianPlacement] << "ns" << std::endl;
}