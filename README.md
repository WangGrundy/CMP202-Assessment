"# CMP202-Assessment" 
"# CMP202-Assessment" 

Wang Grundy | 2101005

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