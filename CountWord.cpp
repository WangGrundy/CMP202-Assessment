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
#include <ctype.h>

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
bool startTimer = false;
int parallelTheadCount = 0;

void ManualTest();
void DefaultTest();
void AdditionalOptions(bool& showThreadsWaiting, bool& showEntireArray, bool& showFirst10ElementsOfArray, std::string& unit);
void Help();

std::barrier bigBarrier(2); //manually change this

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

void GenerateVectorOfWords(std::string word, std::vector<std::string>& vector, int sizeOfVector, int sizeOfRandomWords, int chanceOfWordAppearing) {
	
	for (int i = 0; i < sizeOfVector; i++) {
		
		int randomNum = rand() % chanceOfWordAppearing; //10% chance of our word appearing

		if (randomNum == 1) { //10% chance of our word appearing
			vector.push_back(word);
		}
		else {
			vector.push_back(GenerateRandomString(sizeOfRandomWords)); //90% chance of random string
		}
	}
}

void WordCount(std::string word, const std::vector<std::string>& vector, int& count) {
	
	for (std::string x : vector) { //brute force and search through entire vector (0-size of vector) and the number of custom words added.
		if (x == word) {
			count++;
		}
	}
}

void WordCount_P_Atomic_CV(std::string word, const std::vector<std::string>& vector, int threadTotal, int& count, int threadID) {

	int threadNumber = threadID + 1;
	int threadCountTemp = threadNumber;
	std::unique_lock<std::mutex> uniquelock1(mutexForTimer);

	if (!startTimer) {
		std::cout << "waiting" << std::endl;
		parallelTheadCount++;//add 1 to parallel thread counter
		cv.wait(uniquelock1);
		start1 = the_amp_clock::now(); //start timer
		std::cout << "GO" << std::endl;
	}

	//threads that are still stuck
	startTimer = true;
	cv.notify_all();

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

void WordCount_P_Atomic_Barrier(std::string word, const std::vector<std::string>& vector, int threadTotal, int& count, int threadID) {

	int threadNumber = threadID + 1;
	int threadCountTemp = threadNumber;

	std::unique_lock<std::mutex> uniquelock1(mutexForTimer);

	the_amp_clock::time_point a = the_amp_clock::now();
	bigBarrier.arrive_and_wait(); //all threads wait here then start timer.
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

	startTimer = false;
	std::vector<std::thread> threadVector;
	parallelTheadCount = 0;

	for (int i = 0; i < threadsNum; i++) { //do work
		threadVector.push_back(std::thread(&WordCount_P_Atomic_CV, word, vector, threadsNum, std::ref(count), i));
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

	return duration_cast<milliseconds>(end1 - start1).count(); ///////////////////////////////////////
}

void AskQuestion(int min, int max, int& value, std::string description, std::string recommendation) {
	
	std::cout << "-----------------------------------------------------------------------------" << std::endl;
	while (value < min || value > max) {

		if (std::cin.fail()) { //if we type in string instead of int, we fail.
			std::cin.clear(); //clear input buffer
			std::cin.ignore(1000, '\n'); //input and enterkey gets ignored. ignore/clear characters from the input buffer
			std::cout << "---------------------------------" << std::endl;
			std::cout << "Number invalid, please try again. INT only and within VALID ranges" << std::endl << std::endl;
		}
		
		std::cout << "How many " << description << "? (MIN = " << min << " | MAX = " << max << ")" << std::endl;
		std::cout << recommendation << std::endl;
		std::cout << "CHOICE: ";
		std::cin >> value;
	}
}

void AskQuestion(std::string& word, std::string description, std::string recommendation) {

	std::cout << "-----------------------------------------------------------------------------" << std::endl;
	std::cout << "type in your " << description << " to insert into randomly generated vector: " << std::endl;
	std::cout << recommendation << std::endl;
	std::cout << "CHOICE: ";
	std::cin >> word;
	
}

void AskQuestion(bool& value, std::string description, std::string recommendation) {

	std::string answer = "";

	std::cout << "-----------------------------------------------------------------------------" << std::endl;
	while (true) {
		std::cout << description << std::endl;
		std::cout << "type 'y' for yes, or 'n' for no" << std::endl << std::endl;
		std::cout << recommendation << std::endl;
		std::cout << "CHOICE: ";
		std::cin >> answer;

		if (answer == "y") {
			value = true;
			return;
		}
		else if (answer == "n"){
			value = false;
			return;
		}
	}
}

void AskQuestion(std::string option1, std::string option2, std::string& value, std::string description, std::string recommendation) {

	std::string answer = "";

	std::cout << "-----------------------------------------------------------------------------" << std::endl;
	while (true) {
		std::cout << description << std::endl;
		std::cout << recommendation << std::endl;
		std::cout << "CHOICE: ";
		std::cin >> answer;

		if (answer == option1) {
			value = option1;
			return;
		}
		else if (answer == option2) {
			value = option2;
			return;
		}
	}
}

void ManualTest() {

	int count = 0;
	int count2 = 0;
	std::vector<std::string> vector;
	std::vector<long long> parallelTestTimes, nonParallelTestTimes;

	//questions to ask
	int threadsNum = -1; //should be equal to number of cores - 1 because it includes main thread (advisory)
	std::string word = "hello"; //our word that we will add to the randomly generated vector
	int chanceOfWordAppearing = -1; //10% chance of word appearing in our randomly generated vector
	int sizeOfVector = -1; //the size of how large our vector of words will be to search through
	int sizeOfRandomWords = -1; //size of each word inserted into the vector
	int testRepetitions = -1; //number of times this test will be repeated

	//ask questions
	AskQuestion(2, INT_MAX, threadsNum, "threads", "recommendation: number of cpu's you have on your device");
	AskQuestion(2, INT_MAX, sizeOfVector, "size of vector", "recommendation: 100000 is a good size to test");
	AskQuestion(2, INT_MAX, sizeOfRandomWords, "length of random words", "recommendation: 5-10, keep it short for faster compile time since its randomly generated");
	AskQuestion(0, 100, chanceOfWordAppearing, "chance of words appearing", "recommendation: at least 10% for small vectors");
	AskQuestion(2, INT_MAX, testRepetitions, "test repetitions", "recommendation: if used 100,000 vector word length, 10-20 repetitions should be good");
	AskQuestion(word, "word", "recommendation: any word is fine");

	
	for (int i = 0; i < testRepetitions; i++) {
		GenerateVectorOfWords(word, vector, sizeOfVector, sizeOfRandomWords, chanceOfWordAppearing); //insert our word randomly into this
		std::cout << "starting test[ " << i << " ] " << std::endl;
		nonParallelTestTimes.push_back(TestNonParallel(count, word, sizeOfVector, vector));
		parallelTestTimes.push_back(TestParallel(count2, word, sizeOfVector, vector, threadsNum));
	}

	std::sort(nonParallelTestTimes.begin(), nonParallelTestTimes.end());
	std::sort(parallelTestTimes.begin(), parallelTestTimes.end());

	int medianPlacement = (testRepetitions / 2) - 1;

	/*for (long long num : parallelTestTimes) {
		std::cout << num << std::endl;
	}*/

	std::cout << "SERIAL: COUNT IS : " << count << " AND TIME TAKEN: " << nonParallelTestTimes[medianPlacement] << "ns" << std::endl;
	std::cout << "PARALLEL: COUNT IS : " << count2 << " AND TIME TAKEN: " << parallelTestTimes[medianPlacement] << "ns" << std::endl;
	std::cout << "parallel is faster by " << nonParallelTestTimes[medianPlacement] - parallelTestTimes[medianPlacement] << "ns" << std::endl;
}

void DefaultTest() {
	int count = 0;
	int count2 = 0;
	std::vector<std::string> vector;
	std::vector<long long> parallelTestTimes, nonParallelTestTimes;

	//PLEASE CHANGE TO YOUR LIKING ---------------------------------------
	int threadsNum = cores - 1; //should be equal to number of cores - 1 because it includes main thread (advisory)
	std::string word = "hello"; //our word that we will add to the randomly generated vector
	int chanceOfWordAppearing = 10; //10% chance of word appearing in our randomly generated vector
	int sizeOfVector = 100000; //the size of how large our vector of words will be to search through
	int sizeOfRandomWords = 10; //size of each word inserted into the vector
	int testRepetitions = 10; //number of times this test will be repeated
	//PLEASE CHANGE TO YOUR LIKING ---------------------------------------

	for (int i = 0; i < testRepetitions; i++) {
		GenerateVectorOfWords(word, vector, sizeOfVector, sizeOfRandomWords, chanceOfWordAppearing); //insert our word randomly into this
		std::cout << "starting test[ " << i << " ] " << std::endl;
		nonParallelTestTimes.push_back(TestNonParallel(count, word, sizeOfVector, vector));
		parallelTestTimes.push_back(TestParallel(count2, word, sizeOfVector, vector, threadsNum));
	}

	std::sort(nonParallelTestTimes.begin(), nonParallelTestTimes.end());
	std::sort(parallelTestTimes.begin(), parallelTestTimes.end());

	int medianPlacement = (testRepetitions / 2) - 1;

	/*for (long long num : parallelTestTimes) {
		std::cout << num << std::endl;
	}*/

	std::cout << "SERIAL: COUNT IS : " << count << " AND TIME TAKEN: " << nonParallelTestTimes[medianPlacement] << "ns" << std::endl;
	std::cout << "PARALLEL: COUNT IS : " << count2 << " AND TIME TAKEN: " << parallelTestTimes[medianPlacement] << "ns" << std::endl;
	std::cout << "parallel is faster by " << nonParallelTestTimes[medianPlacement] - parallelTestTimes[medianPlacement] << "ns" << std::endl;
}

void AdditionalOptions(bool& showThreadsWaiting, bool& showEntireArray, bool& showFirst10ElementsOfArray, std::string& unit) {
	AskQuestion(showThreadsWaiting, "show threads waiting?", "recommendation: say no if you have a super large number of threads (default no)");
	AskQuestion(showEntireArray, "show entire generated array?", "recommendation: no, if you have large arrays and test repetitions (default no)");
	AskQuestion(showFirst10ElementsOfArray, "show first 10 elements of randomly generated array?", "recommendation: no, if you have large arrays and test repetitions (default no)");
	AskQuestion("n", "m", unit, "answer shown in miliseconds type (m) or nanoseconds type (n)?", "recommendation: depends on how large array is, if large array maybe m for miliseconds is good enough");
}

void Help() {

}

int main() {
	srand(time(0));
	cores = std::thread::hardware_concurrency();
	std::cout << "number of cores: " << cores << std::endl;

	//PLEASE CHANGE OPTIONS HERE IF YOU FIND IT EASIER 
	bool showThreadsWaiting = false;
	bool showEntireArray = false;
	bool showFirst10ElementsOfArray = false;
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
			ManualTest();
			break;
		case 2:
			DefaultTest();
			break;
		case 3:
			AdditionalOptions(showThreadsWaiting, showEntireArray, showFirst10ElementsOfArray, unit);
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

	/*for (std::string word : vector) {
		std::cout << word << std::endl;
	}*/

	/*for (long long num : parallelTestTimes) {
		std::cout << num << std::endl;
	}*/
}