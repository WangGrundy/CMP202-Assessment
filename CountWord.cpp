#include <iostream>
#include <amp.h>
#include <thread>
#include <mutex>

#include <chrono>
#include <iomanip>
#include <time.h>
#include <array>
#include <numeric>

using namespace concurrency;
using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::chrono::nanoseconds;
typedef std::chrono::steady_clock the_serial_clock;
typedef std::chrono::steady_clock the_amp_clock;

bool timerStarted = false;
the_amp_clock::time_point start1;
the_amp_clock::time_point end1;

std::atomic_int elementsDoneAtomic = -1;
std::atomic_int countAtomic = 0;

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

void WordCount_P_Atomic(std::string word, const std::vector<std::string>& vector, int threadsNum, int& count) {

	auto claim_index = []() -> int { //each thread can "claim" some spot in the vector using the value returned by fetch_add
		return elementsDoneAtomic.fetch_add(1, std::memory_order_seq_cst); //add 1 to element checked counter  //single total order exists in which all threads observe all modifications in the same order with {memory_order_seq_cst}
	};  // returns the value immediately preceding the effects of this function in the modification order of *this, AKA elementsDoneAtomic + 1

	int claimed_index;

	//if (!timerStarted) {
	//	timerStarted = true;
	//	start1 = the_amp_clock::now(); //start timer
	//	
	//}

	//start1 = the_amp_clock::now(); //start timer

	while ((claimed_index = claim_index()) < vector.size()) {
		std::cout << claimed_index << std::endl;

		//if a word in the vector is equal to the word we are looking for,
		if (vector[claimed_index] == word) {
			countAtomic++; //add 1 to the word count
		}
	}

	end1 = the_amp_clock::now(); //end timer
	count = countAtomic; //set the count with the atomic count
}

void concurrent_method_timer()
{
	// WHEN ONE SIGNAL START COMES IN:
		// START TIMING

	// WHEN ALL SIGNAL STOPS COME IN:
		// END TIMER.

	// RETURN TIME

}

void WordCount_P_Atomic_2(std::string word, const std::vector<std::string>& vector, int threadTotal, int& count, int thread) {



	// SIGNAL START


	int threadNumber = thread + 1; //doesn't account for the 0th element; //rework this
	int threadCountTemp = threadNumber;

	//if (!timerStarted) { 
	//	timerStarted = true;
	//	start1 = the_amp_clock::now(); //start timer

	//}
	start1 = the_amp_clock::now(); //start timer
	
	while (threadCountTemp < vector.size()) {

		if (vector[threadCountTemp] == word) {
			countAtomic++;
		}

		threadCountTemp += threadTotal;
	}

	// SIGNAL END

	//end1 = the_amp_clock::now(); //end timer
	count = countAtomic; //set the count with the atomic count
}

long long TestNonParallel(int& count, std::string word, int sizeOfVector, const std::vector<std::string>& vector) {
	
	the_amp_clock::time_point start = the_amp_clock::now(); ///////////////////////////////////////
	WordCount(word, vector, count);
	the_amp_clock::time_point end = the_amp_clock::now();
	return duration_cast<nanoseconds>(end - start).count(); ///////////////////////////////////////
}

long long TestParallel(int& count, std::string word, int sizeOfVector, std::vector<std::string>& vector, int threadsNum) {
	
	std::mutex count_mutex;
	std::mutex elementsDone_mutex;
	int elementsDone = 0;
	std::vector<std::thread> threadVector;

	for (int i = 0; i < threadsNum; i++) {
		threadVector.push_back(std::thread(&WordCount_P_Atomic_2, word, vector, threadsNum, std::ref(count), i));
	}

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
	int sizeOfVector = 100;
	int sizeOfRandomWords = 5;
	std::vector<std::string> vector;

	GenerateVectorOfWords(word, vector, sizeOfVector, sizeOfRandomWords);
	for (std::string word : vector) {
		std::cout << word << std::endl;
	}

	long long time_taken = TestNonParallel(count, word, sizeOfVector, vector);
	long long time_taken2 = TestParallel(count2, word, sizeOfVector, vector, threadsNum);

	std::cout << "[1]SERIAL: COUNT IS : " << count << " AND TIME TAKEN: " << std::setprecision(7) << time_taken << "ms" << std::endl;
	std::cout << "[1]PARALLEL: COUNT IS : " << count2 << " AND TIME TAKEN: "<< std::setprecision(7) << time_taken2 << "ms" << std::endl;

	std::cout << "starting second test" << std::endl;

	timerStarted = false;
	elementsDoneAtomic = -1;
	std::atomic_int countAtomic = 0;

	/*time_taken = TestNonParallel(count, word, sizeOfVector, vector);
	time_taken2 = TestParallel(count2, word, sizeOfVector, vector, threadsNum);

	std::cout << "[2]SERIAL: COUNT IS : " << count << " AND TIME TAKEN: " << std::setprecision(7) << time_taken << "ms" << std::endl;
	std::cout << "[2]PARALLEL: COUNT IS : " << count2 << " AND TIME TAKEN: " << std::setprecision(7) << time_taken2 << "ms" << std::endl;*/





	/*std::string test;
	char abc[] = "ABC";
	test.reserve(2);
	test += abc[0]; test += abc[2];
	std::cout << test;*/

}