//Wang Yu Grundy | 2101005 , CMP 203 Data Structures & Algorithms 2 Project (2023)
#define _SILENCE_AMP_DEPRECATION_WARNINGS

#include <iostream>
#include <amp.h>
#include <thread>
#include <mutex>
#include <chrono>
#include <time.h>
#include <array>
#include <numeric>
#include <algorithm>
#include <condition_variable>
#include <barrier>

#include "CountWordHelperFunctions.h"

using namespace concurrency;
using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::chrono::nanoseconds;
typedef std::chrono::steady_clock the_serial_clock;
typedef std::chrono::steady_clock the_amp_clock;

bool timerStarted = false;
the_amp_clock::time_point start_Parallel;
the_amp_clock::time_point end_Parallel;
the_amp_clock::time_point end_CV;
the_amp_clock::time_point start_CV;
the_amp_clock::time_point end_Barrier;
the_amp_clock::time_point start_Barrier;
std::atomic_int countAtomic_CV = 0;
std::atomic_int countAtomic_Barrier = 0;
std::condition_variable cv;
std::mutex mutexForTimer;
int cores = 0;
bool startTimer = false;
int parallelTheadCount = 0;

void ManualTest(const bool& showThreadsWaiting, const bool& showEntireArray, const bool& showFirst10ElementsOfArray, const std::string& unit, const bool& testNumber, bool& showTestTimes);
void DefaultTest(const bool& showThreadsWaiting, const bool& showEntireArray, const bool& showFirst10ElementsOfArray, const std::string& unit, const bool& testNumber, bool& showTestTimes);
void AdditionalOptions(const bool& showThreadsWaiting, const bool& showEntireArray, const bool& showFirst10ElementsOfArray, const std::string& unit, const bool& testNumber, bool& showTestTimes);

//manually change this ************************************************************************************************************************************
std::barrier bigBarrier(5); //number inside brackets should be number of threads used (default: number of cores - 1) 
bool disableBarrierTest = false;
//manually change this ************************************************************************************************************************************

void WordCount(const std::string& word, const std::vector<std::string>& vector, int& count) {
	start_Parallel = the_amp_clock::now();
	for (const std::string& x : vector) { //brute force and search through entire vector (0-size of vector) and the number of custom words added.
		if (x == word) {
			count++;
		}
	}
	end_Parallel = the_amp_clock::now();
}

void WordCount_P_Atomic_CV(std::string word, const std::vector<std::string>& vector, int threadTotal, int& count, int threadID, const bool showThreadsWaiting) {

	int threadNumber = threadID + 1;
	int threadCountTemp = threadNumber;
	std::unique_lock<std::mutex> uniquelock1(mutexForTimer);

	if (!startTimer) {
		if (showThreadsWaiting) {
			std::cout << "waiting" << std::endl;
		}
		parallelTheadCount++;//add 1 to parallel thread counter
		cv.wait(uniquelock1); //wait here for all threads to be ready
		if (showThreadsWaiting) {
			std::cout << "GO" << std::endl;
		}
		start_CV = the_amp_clock::now(); //start timer
	}

	//threads that are still stuck
	startTimer = true;
	cv.notify_all();

	if (threadNumber == 1) { //account for the 0th element if 1st thread
		if (vector[0] == word) {
			countAtomic_CV++;
		}
	}

	while (threadCountTemp < vector.size()) {

		if (vector[threadCountTemp] == word) {
			countAtomic_CV++;
		}

		threadCountTemp += threadTotal;
	}

	end_CV = the_amp_clock::now(); //end timer
	count = countAtomic_CV; //set the count with the atomic count
}

void WordCount_P_Atomic_Barrier(std::string word, const std::vector<std::string>& vector, int threadTotal, int& count, int threadID) {
	
	int threadNumber = threadID + 1;
	int threadCountTemp = threadNumber;

	bigBarrier.arrive_and_wait(); //all threads wait here then start timer.
	start_Barrier = the_amp_clock::now(); //start timer

	if (threadNumber == 1) { //account for the 0th element if 1st thread
		if (vector[0] == word) {
			countAtomic_Barrier++;
		}
	}

	while (threadCountTemp < vector.size()) {
		
		if (vector[threadCountTemp] == word) {
			countAtomic_Barrier++;
		}

		threadCountTemp += threadTotal;
	}

	end_Barrier = the_amp_clock::now(); //end timer
	count = countAtomic_Barrier; //set the count with the atomic count
}

long long TestNonParallel(int& count, const std::string& word, const int& sizeOfVector, const std::vector<std::string>& vector, const std::string& unit) {
	WordCount(word, vector, count);
	return ReturnUnit(unit, start_Parallel, end_Parallel);
}

long long TestParallel_CV(int& count, const std::string& word, const int& sizeOfVector, const std::vector<std::string>& vector, int threadsNum, const bool& showThreadsWaiting, const std::string& unit) {

	startTimer = false;
	std::vector<std::thread> threadVector;
	parallelTheadCount = 0;

	for (int i = 0; i < threadsNum; i++) { //do work
		threadVector.push_back(std::thread(&WordCount_P_Atomic_CV, word, vector, threadsNum, std::ref(count), i, showThreadsWaiting));
	}
	
	while (parallelTheadCount != threadsNum) { //busy waiting
		//do nothing if threads aren't ready to be start timer
	}
	//threads should be ready at this point
	startTimer = true;
	cv.notify_all(); //notify all threads that thread creation has been complete

	for (int i = 0; i < threadsNum; i++) {
		threadVector[i].join();
	}

	return ReturnUnit(unit, start_CV, end_CV);
}

long long TestParallel_Barrier(int& count, const std::string& word, const int& sizeOfVector, const std::vector<std::string>& vector, int threadsNum, const std::string& unit) {

	startTimer = false;
	std::vector<std::thread> threadVector;
	parallelTheadCount = 0;

	for (int i = 0; i < threadsNum; i++) { //do work
		threadVector.push_back(std::thread(&WordCount_P_Atomic_Barrier, word, vector, threadsNum, std::ref(count), i));
	}

	for (int i = 0; i < threadsNum; i++) {
		threadVector[i].join();
	}

	return ReturnUnit(unit, start_Barrier, end_Barrier);
}

void ManualTest(const bool& showThreadsWaiting, const bool& showEntireArray, const bool& showFirst10ElementsOfArray, const std::string& unit, const bool& testNumber, bool& showTestTimes) {

	int countNonParallel = 0, countCV = 0, countBarrier = 0;
	std::vector<long long> parallelTestTimes_CV, parallelTestTimes_Barrier, nonParallelTestTimes;

	//questions to ask
	int threadsNum = -1; //should be equal to number of cores - 1 because it includes main thread (advisory)
	std::string word = "hello"; //our word that we will add to the randomly generated vector
	int chanceOfWordAppearing = -1; //10% chance of word appearing in our randomly generated vector
	int sizeOfVector = -1; //the size of how large our vector of words will be to search through
	int sizeOfRandomWords = -1; //size of each word inserted into the vector
	int testRepetitions = -1; //number of times this test will be repeated

	//ask questions
	AskQuestion(2, INT_MAX, threadsNum, "threads", "recommendation: number of cpu's you have on your device - 1 (default: cores - 1)");
	AskQuestion(2, INT_MAX, sizeOfVector, "size of vector", "recommendation: 100,000 is a good size to test  (default: 100000)");
	AskQuestion(2, INT_MAX, sizeOfRandomWords, "length of random words", "recommendation: 5-20, keep it short for faster compile time since its randomly generated (default: 10)");
	AskQuestion(0, 100, chanceOfWordAppearing, "chance of words appearing", "recommendation: at least 10% for small vectors (default: 10)");
	AskQuestion(2, INT_MAX, testRepetitions, "test repetitions", "recommendation: 10-20 repetitions should be good (default: 10)");
	AskQuestion(word, "word", "recommendation: any word is fine (default: Hello)");

	for (int i = 0; i < testRepetitions; i++) {
		std::vector<std::string> vector;

		GenerateVectorOfWords(word, vector, sizeOfVector, sizeOfRandomWords, chanceOfWordAppearing); //insert our word randomly into this
		ShowFirst10ElementsOrWhole(showFirst10ElementsOfArray, showEntireArray, vector);
		ShowTestNumber(testNumber, i);
		
		//create threads
		nonParallelTestTimes.push_back(TestNonParallel(countNonParallel, word, sizeOfVector, vector, unit));
		parallelTestTimes_CV.push_back(TestParallel_CV(countCV, word, sizeOfVector, vector, threadsNum, showThreadsWaiting, unit));
		if (!disableBarrierTest) {
			parallelTestTimes_Barrier.push_back(TestParallel_Barrier(countBarrier, word, sizeOfVector, vector, threadsNum, unit));
		}
	}

	//sort test time vectors
	std::sort(nonParallelTestTimes.begin(), nonParallelTestTimes.end());
	std::sort(parallelTestTimes_CV.begin(), parallelTestTimes_CV.end());
	if (!disableBarrierTest) {
		std::sort(parallelTestTimes_Barrier.begin(), parallelTestTimes_Barrier.end());
	}

	ShowTestTimes(showTestTimes, nonParallelTestTimes, parallelTestTimes_CV, parallelTestTimes_Barrier, disableBarrierTest);
	ShowSummary(unit, nonParallelTestTimes, parallelTestTimes_CV, parallelTestTimes_Barrier, testRepetitions, countNonParallel, countCV, countBarrier, disableBarrierTest);
}

void DefaultTest(const bool& showThreadsWaiting, const bool& showEntireArray, const bool& showFirst10ElementsOfArray, const std::string& unit, const bool& testNumber, bool& showTestTimes) {
	int countNonParallel = 0, countCV = 0, countBarrier = 0;
	std::vector<long long> parallelTestTimes_CV, parallelTestTimes_Barrier, nonParallelTestTimes;

	//PLEASE CHANGE TO YOUR LIKING ---------------------------------------
	int threadsNum = 5; // cores - 1; //should be equal to number of cores - 1 because it includes main thread (advisory)
	std::string word = "hello"; //our word that we will add to the randomly generated vector
	int chanceOfWordAppearing = 10; //10% chance of word appearing in our randomly generated vector
	int sizeOfVector = 100000; //the size of how large our vector of words will be to search through
	int sizeOfRandomWords = 10; //size of each word inserted into the vector
	int testRepetitions = 10; //number of times this test will be repeated
	//PLEASE CHANGE TO YOUR LIKING ---------------------------------------

	for (int i = 0; i < testRepetitions; i++) {
		std::vector<std::string> vector;

		GenerateVectorOfWords(word, vector, sizeOfVector, sizeOfRandomWords, chanceOfWordAppearing); //insert our word randomly into this
		ShowFirst10ElementsOrWhole(showFirst10ElementsOfArray, showEntireArray, vector);
		ShowTestNumber(testNumber, i);

		//create threads
		nonParallelTestTimes.push_back(TestNonParallel(countNonParallel, word, sizeOfVector, vector, unit));
		parallelTestTimes_CV.push_back(TestParallel_CV(countCV, word, sizeOfVector, vector, threadsNum, showThreadsWaiting, unit));
		if (!disableBarrierTest) {
			parallelTestTimes_Barrier.push_back(TestParallel_Barrier(countBarrier, word, sizeOfVector, vector, threadsNum, unit));
		}
	}

	//sort test time vectors
	std::sort(nonParallelTestTimes.begin(), nonParallelTestTimes.end());
	std::sort(parallelTestTimes_CV.begin(), parallelTestTimes_CV.end());
	if (!disableBarrierTest) {
		std::sort(parallelTestTimes_Barrier.begin(), parallelTestTimes_Barrier.end());
	}
	
	ShowTestTimes(showTestTimes, nonParallelTestTimes, parallelTestTimes_CV, parallelTestTimes_Barrier, disableBarrierTest);
	ShowSummary(unit, nonParallelTestTimes, parallelTestTimes_CV, parallelTestTimes_Barrier, testRepetitions, countNonParallel, countCV, countBarrier, disableBarrierTest);
}

void AdditionalOptions(bool& showThreadsWaiting, bool& showEntireArray, bool& showFirst10ElementsOfArray, std::string& unit, bool& testNumber, bool& showTestTimes) {
	AskQuestion(showThreadsWaiting, "show threads waiting?", "recommendation: say no if you have a super large number of threads (default no)");
	AskQuestion(showEntireArray, "show entire generated array?", "recommendation: no, if you have large arrays and test repetitions (default no)");
	AskQuestion(showFirst10ElementsOfArray, "show first 10 elements of randomly generated array?", "recommendation: no, if you have large arrays and test repetitions (default no)");
	AskQuestion(testNumber, "Show test number?", "recommendation: depends on how large your test sample is (default yes)");
	AskQuestion(showTestTimes, "Show ALL test times at the end?", "recommendation: depends on how large your test sample, this will enable you to check for correctness (default no)");
	AskQuestion("n", "m", unit, "answer shown in miliseconds type (m) or nanoseconds type (n)?", "recommendation: depends on how large array is, if large array maybe m for miliseconds is good enough (default n)");
}

int main() {
	srand(time(0));
	cores = std::thread::hardware_concurrency();
	std::cout << "number of cores: " << cores << std::endl;

	//PLEASE CHANGE OPTIONS HERE IF YOU FIND IT EASIER 
	bool showThreadsWaiting = false; //show threads waiting for condition variable test
	bool showEntireArray = false; //show entire vector
	bool showFirst10ElementsOfArray = false; //show first 10 elements of vector
	bool testNumber = true; //show test number when running
	bool showTestTimes = false; //show test time array at the end (includes all results)
	std::string unit = "n"; //"m" for miliseconds or "n" for nanoseconds
	//PLEASE CHANGE OPTIONS HERE IF YOU FIND IT EASIER 

	while (true) {
		int optionNumber = -1;
		while (optionNumber < 1 || optionNumber > 5) {

			if (std::cin.fail()) { //if we type in string instead of int, we fail.
				std::cin.clear(); //clear input buffer
				std::cin.ignore(1000, '\n'); //input and enterkey gets ignored. ignore/clear characters from the input buffer
				std::cout << "---------------------------------" << std::endl;
				std::cout << "Number invalid, please try again." << std::endl << std::endl;
			}

			std::cout << std::endl;
			std::cout << "please select an option: (1-5)" << std::endl;
			std::cout << "1. manaual test" << std::endl;
			std::cout << "2. default test" << std::endl;
			std::cout << "3. additional options" << std::endl;
			std::cout << "4. help (more info for each option)" << std::endl;
			std::cout << "5. quit" << std::endl;
			std::cout << "CHOICE: ";
			std::cin >> optionNumber;
		}

		switch (optionNumber) {
		case 1:
			ManualTest(showThreadsWaiting, showEntireArray, showFirst10ElementsOfArray, unit, testNumber, showTestTimes);
			break;
		case 2:
			DefaultTest(showThreadsWaiting, showEntireArray, showFirst10ElementsOfArray, unit, testNumber, showTestTimes);
			break;
		case 3:
			AdditionalOptions(showThreadsWaiting, showEntireArray, showFirst10ElementsOfArray, unit, testNumber, showTestTimes);
			break;
		case 4:
			Help();
			break;
		case 5:
			return 0;
		default:
			std::cout << "something went wrong" << std::endl;
		}
	}
}