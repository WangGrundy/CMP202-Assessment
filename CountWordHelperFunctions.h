#pragma once
#include <iostream>

using namespace concurrency;
using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::chrono::nanoseconds;
typedef std::chrono::steady_clock the_serial_clock;
typedef std::chrono::steady_clock the_amp_clock;

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

void AskQuestion(int min, int max, int& value, std::string description, std::string recommendation) {

	while (value < min || value > max) {
		std::cout << "-----------------------------------------------------------------------------" << std::endl;
		if (std::cin.fail()) { //if we type in string instead of int, we fail.
			std::cout << "ERROR -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.- type valid answer " << std::endl;
			std::cin.clear(); //clear input buffer
			std::cin.ignore(1000, '\n'); //input and enterkey gets ignored. ignore/clear characters from the input buffer
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

	while (true) {
		std::cout << "-----------------------------------------------------------------------------" << std::endl;
		std::cout << description << std::endl;
		std::cout << "type 'y' for yes, or 'n' for no" << std::endl;
		std::cout << recommendation << std::endl;
		std::cout << "CHOICE: ";
		std::cin >> answer;

		if (answer == "y") {
			value = true;
			return;
		}
		else if (answer == "n") {
			value = false;
			return;
		}
		else {
			std::cout << "ERROR -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.- type valid answer " << std::endl;
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

void ShowFirst10ElementsOrWhole(const bool& firstTen, const bool& whole,const std::vector<std::string>& vector) {
	if (firstTen) { //print first 10 elements if enabled
		for (int i = 0; i < 10; i++) {
			std::cout << vector[i] << std::endl;
		}
	}
	else if (whole) { //print entire vector if enabled
		for (std::string element : vector) {
			std::cout << element << std::endl;
		}
	}
}

void ShowTestNumber(const bool& testNumber,const int& i) {
	if (testNumber) { //if enabled
		std::cout << "starting test[ " << i << " ] " << std::endl;
	}
}

void ShowTestTimes(const bool& showTestTimes, const std::vector<long long>& nonParallelTestTimes, const std::vector<long long>& parallelTestTimes_CV, const std::vector<long long>& parallelTestTimes_Barrier, const bool& disableBarrierTest) {
	if (showTestTimes) {
		std::cout << "---------------------------------------------------------" << std::endl;
		std::cout << "NON PARALLEL TEST TIMES: " << std::endl;
		for (long long num : nonParallelTestTimes) {
			std::cout << num << std::endl;
		}
		std::cout << "---------------------------------------------------------" << std::endl;
		std::cout << "PARALLEL (condition variables) TEST TIMES: " << std::endl;
		for (long long num : parallelTestTimes_CV) {
			std::cout << num << std::endl;
		}
		if (!disableBarrierTest) {
			std::cout << "---------------------------------------------------------" << std::endl;
			std::cout << "PARALLEL (barrier) TEST TIMES: " << std::endl;
			for (long long num : parallelTestTimes_Barrier) {
				std::cout << num << std::endl;
			}
		}
		std::cout << "---------------------------------------------------------" << std::endl;
	}
}

void ShowSummary(const std::string& unit, const std::vector<long long>& nonParallelTestTimes, const std::vector<long long>& parallelTestTimes_CV, const std::vector<long long>& parallelTestTimes_Barrier, const int& testRepetitions,const int& countNonParallel, const int& countCV, const int& countBarrier, const bool& disableBarrierTest) {
	int medianPlacement = (testRepetitions / 2) - 1;

	std::string typeOfUnit = "";
	if (unit == "m") {
		typeOfUnit = " miliseconds";
	}
	else if (unit == "n") {
		typeOfUnit = " nanoseconds";
	}

	std::cout << "[NON PARALLEL] number of our words is: " << countNonParallel << " and AVERAGE time taken: " << nonParallelTestTimes[medianPlacement] << typeOfUnit << std::endl;
	std::cout << "[PARALLEL(condition variable)] number of our words is: " << countCV << " and AVERAGE time taken: " << parallelTestTimes_CV[medianPlacement] << typeOfUnit << std::endl;
	if (!disableBarrierTest) {
		std::cout << "[PARALLEL(barrier)] number of our words is: " << countBarrier << " and AVERAGE time taken: " << parallelTestTimes_Barrier[medianPlacement] << typeOfUnit << std::endl << std::endl;
	}
	std::cout << "parallel (condition variable) is faster than non parallel by " << nonParallelTestTimes[medianPlacement] - parallelTestTimes_CV[medianPlacement] << typeOfUnit << std::endl;
	if (!disableBarrierTest) {
		std::cout << "parallel (barrier) is faster than non parallel by " << nonParallelTestTimes[medianPlacement] - parallelTestTimes_Barrier[medianPlacement] << typeOfUnit << std::endl;
	}
}

long long ReturnUnit(const std::string& unit, the_amp_clock::time_point start, the_amp_clock::time_point end) {
	if (unit == "m") {
		return duration_cast<milliseconds>(end - start).count(); ///////////////////////////////////////
	}
	else if (unit == "m") {
		return duration_cast<nanoseconds>(end - start).count(); ///////////////////////////////////////
	}
	else { //shouldn't ever come to this
		return duration_cast<nanoseconds>(end - start).count(); ///////////////////////////////////////
	}
}

void Help() {
	std::cout << R"(
WARNING!(PROJECT WON'T WORK IF BARRIER NUMBER IS NOT MANUALLY SET TO THREAD NUMBER) - The project will get stuck. _____________________________________________________________________________________________________________________________________________________________
Barrier won't work in CountWord.cpp if barrier is not equal to number of threads used (please set manually to number of threads being used (line 44))
if using the option 2 "default" please set it to 20.
I couldn't find a way to make it dynamically set - sorry.
[If problem persists, use the safety feature in CountWord.cpp: disableBarrierTest bool and set it to true in CountWord.cpp (line 45). This will disable the barrier test and only use condition variables]
[or you can go to additional options and disable barrier testing from there]

What my code does: _____________________________________________________________________________________________________________________________________________________________
1. Generate a vector of strings randomly.
2. Insert our word in vector a random amount of times.
3. Use threads to count the number of our words that we inserted into the vector.
4. Print out time taken for non parallel method and parallel methods to achieve goal

How to navigate the main menu: _____________________________________________________________________________________________________________________________________________________________
option 1. manual test - Manually input numbers and variables into test. (*This is a bit annoying to use, but follow the recommended inputs when asked for input if stuck or if test bugs out)
(you must change bigBarrier (line44) to equal thread number inserted)

option 2. default test - Automatically have numbers, variables and threads added for you.
(I recommend you use this and change variables inside DefaultTest() in CountWord.cpp where it says in comments: "//PLEASE CHANGE TO YOUR LIKING ---------------------------------------" (line 225)

option 3. Additional options - this will provide you with additional things to check for correctness. (follow the recommended inputs when asked for input if stuck or if test bugs out)
(this can also manually edited in main() in CountWord.cpp starting on line 275)
please enable 1 thing at a time otherwise it gets very confusing to look at in the console.

other options self explanitory.

How to get test results?  _____________________________________________________________________________________________________________________________________________________________
type option 2 and press enter from main menu, it should start the test and print results when done.
or
type option 1 and manually set variables, it should also start the test when variables added and print results when done.
(keep in mind that test results print median time taken result)

Additional info:_____________________________________________________________________________________________________________________________________________________________
The project works with:
Platform toolset: Visual Studio 2022 (v143)
c++ version: ISO C++20 Standard (/std:c++20)
using debug mode because barriers don't work on release.

main project files: CountWord.cpp, CountWordHelperFunctions.h
I have also have another file parallel.cpp which adds two vectors, but uses GPU but isn't really part of my main project. But you have to comment out CountWord.cpp to use it and uncomment Parallel.cpp.
	)";
}