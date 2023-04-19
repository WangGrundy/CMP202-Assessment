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

std::atomic_int elementsDoneAtomic = 0;
std::atomic_int countAtomic = 0;

void GenerateVectorOfWords(std::string word, std::vector<std::string>& vector, int size) {
	
	for (int i = 0; i < size; i++) {
		
		int randomNum = rand() % 10;

		if (randomNum == 1) {
			vector.push_back(word);
		}
		else {
			int randomNum = rand() % 126 + 32;
			char charTemp = (char)randomNum;
			vector.emplace_back(1, charTemp); //forwards the arguments to a constructor (size, char)
		}
	}
}

void WordCount(std::string word, const std::vector<std::string>& vector, int& count) {

	for (std::string x : vector) {
		if (x == word) {
			count++;
		}
	}
}

void WordCount_P(std::string word, const std::vector<std::string>& vector, int threadsNum, int& count, int& elementsDone, std::mutex& count_mutex, std::mutex& elementsDone_mutex) {
	
	if (!timerStarted) {
		timerStarted = true;
		start1 = the_amp_clock::now();
	}

	//increment elementDone so other threads can do their thing
	while (elementsDone < vector.size()) {
		elementsDone_mutex.lock();
		elementsDone++;
		elementsDone_mutex.unlock();


		if (vector[elementsDone - 1] == word) {
			count_mutex.lock();
			count++;
			count_mutex.unlock();
		}
	}

	end1 = the_amp_clock::now();
}

void WordCount_P_Atomic(std::string word, const std::vector<std::string>& vector, int threadsNum, int& count) {

	if (!timerStarted) {
		timerStarted = true;
		start1 = the_amp_clock::now();
	}

	//increment elementDone so other threads can do their thing
	while (elementsDoneAtomic < vector.size()) { //not thread safe
		elementsDoneAtomic++;
		if (vector[elementsDoneAtomic - 1] == word) { //not thread safe
			countAtomic++;
		}
	}

	count = countAtomic;
	end1 = the_amp_clock::now();
}

long long TestNonParallel(int& count, std::string word, int size, const std::vector<std::string>& vector) {
	
	the_amp_clock::time_point start = the_amp_clock::now(); ///////////////////////////////////////
	WordCount(word, vector, count);
	the_amp_clock::time_point end = the_amp_clock::now();
	return duration_cast<milliseconds>(end - start).count(); ///////////////////////////////////////
}

long long TestParallel(int& count, std::string word, int size, std::vector<std::string>& vector, int threadsNum) {
	
	std::mutex count_mutex;
	std::mutex elementsDone_mutex;
	int elementsDone = 0;
	std::vector<std::thread> threadVector;

	//the_amp_clock::time_point start = the_amp_clock::now(); ///////////////////////////////////////
	for (int i = 0; i < threadsNum; i++) {
		//threadVector.push_back(std::thread(&WordCount_P, word, vector, threadsNum, std::ref(count), std::ref(elementsDone), std::ref(count_mutex), std::ref(elementsDone_mutex)));
		threadVector.push_back(std::thread(&WordCount_P_Atomic, word, vector, threadsNum, std::ref(count)));
	}

	for (int i = 0; i < threadsNum; i++) {
		threadVector[i].join();
	}

	//the_amp_clock::time_point end = the_amp_clock::now();  
	return duration_cast<milliseconds>(end1 - start1).count(); ///////////////////////////////////////
}

int main() {
	srand(time(0));

	std::string word = "hello";
	int count = 0;
	int count2 = 0;
	int cores = std::thread::hardware_concurrency();
	int threadsNum = cores - 1;
	std::cout << "number of cores: " << cores << std::endl;
	int size = 10000000;
	std::vector<std::string> vector;

	GenerateVectorOfWords(word, vector, size);
	/*for (std::string word : vector) {
		std::cout << word << std::endl;
	}*/

	long long time_taken = TestNonParallel(count, word, size, vector);
	long long time_taken2 = TestParallel(count2, word, size, vector, threadsNum);

	std::cout << "SERIAL: COUNT IS : " << count << " AND TIME TAKEN: " << std::setprecision(7) << time_taken << "ms" << std::endl;
	std::cout << "PARALLEL: COUNT IS : " << count2 << " AND TIME TAKEN: "<< std::setprecision(7) << time_taken2 << "ms" << std::endl;
}