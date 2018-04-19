/*
 * Current Issues:
 *
 *  Switch from recursive to itterative
 *
 *  bfsGuidedPathGen is very slow to return a path failure. Maybe make new BFS_exists funciton to only
 *  call gen when a path exists.
 *
 * Unreachable nodes?
 *
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

string defaultFilepath = "../benchmark_files/testModel.blif";


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


    // run the FBB
    int result = FBB_base(solutionRatioTarget, solutionDeviation, filePath);

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

