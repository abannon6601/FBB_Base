/*
 * Current Issues:
 *
 *  BFS path gen is generating ghost nodes for s9234
 *
 *  Basically s-xyz files are weird and my parser doesn't like them
 *  check inputs; may not be tagging properly
 */
/*
 * Code Structure:
 *
 *
 *
 */

#include <iostream>
#include <fstream>
#include <map>
#include <sstream>
#include <vector>
#include <algorithm>
#include <list>
#include <queue>
#include <math.h>
#include <ctime>
#include <random>

#include "Hmain.h"



using namespace std;

//FUNCTION DELCRATIONS
int fetchInt();
bool fetchYN();

string defaultFilepath = "../benchmark_files/testModel.blif"; // used if no user filepath given


int main() {

    string filePath;
    float solutionRatioTarget;
    float solutionDeviation;
    int inputNum;

    std::cout <<"FBB-Launcher: Please enter filepath: ";
    std::getline(std::cin, filePath);

    if(filePath.length() < 1)
    {
        filePath = defaultFilepath;
        std::cout <<"FBB-Launcher: No filepath entered, using default filepath: " + filePath << endl;
    }

    std::cout <<"FBB-Launcher: Please enter target solution ratio (enter a number between 1-100):";
    inputNum = fetchInt();
    while(inputNum < 0 || inputNum > 100)
    {
        std::cout <<"FBB-Launcher: Please enter a valid percentage ratio:";
        inputNum = fetchInt();
    }

    solutionRatioTarget = (float) inputNum/100;

    std::cout <<"FBB-Launcher: Please enter target solution deviation (enter a number between 1-100):";
    inputNum = fetchInt();
    while(inputNum < 0 || inputNum > 100)
    {
        std::cout <<"FBB-Launcher: Please enter a valid percentage ratio:";
        inputNum = fetchInt();
    }

    solutionDeviation = (float) inputNum/100;

    std::cout <<"FBB-Launcher: Enable verbose output y/n?:";
    bool verbose = fetchYN();


    // run the FBB
    int result = FBB_base(solutionRatioTarget, solutionDeviation, filePath, verbose);

    if(result != 0)
        std::cout << "FBB-Partitioner: Failed with error: " << result << std::endl;

    return result;

}

//Fetches an integer from the user and avoids crashes when char string is input into scanf
int fetchInt()
{
    int inputInt;
    bool foundInt = false;
    int result;

    while(!foundInt)
    {
        result = scanf("%d",&inputInt);
        if(result == 0)
        {
            while (fgetc(stdin) != '\n');   // clear the rest of the input buffer
            inputInt = 0;                   // clear junk from the input
            printf("Please enter valid integer\n");
        }
        else
        {
            foundInt = true;
        }
    }
    return inputInt;
}

//Fetches a true/false form the user with y/n
bool fetchYN()
{
    string userInput;

    bool flowControl = false;
    while(!flowControl)
    {
        std::getline(std::cin, userInput);

        if(userInput == "Y" || userInput == "y")
        {
            return true;
        }
        if(userInput == "N" || userInput == "n")
        {
            return false;
        }
    }
}
