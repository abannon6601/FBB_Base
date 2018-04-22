#include <iostream>
#include <fstream>
#include <map>
#include <sstream>
#include <vector>
#include <algorithm>
#include "Hmain.h"
using namespace std;

// TODO update latch parsing to remove additional type-control options if present

// Function to parse BLIF files into an ordered hashmap wih the names of the gates as the key
map<string, node> readFile(string fileAdr){

    string line;
    string buf;
    std::map<std::string, node> nodeMap;
    vector<string> inputs;
    vector<string> outputs;

    ifstream myfile(fileAdr);
    if (myfile.is_open())
    {
        while (getline (myfile,line))   // iterate down the entire file
        {
            if(line.empty())
                continue; // skip empty lines (prevents crash on empty line)

            // splint each line into words
            stringstream ss(line);
            vector<string> words;
            while (ss >> buf)
                words.push_back(buf);   // words is now a vector of the elements in line



            if(words[0] == ".names" || words[0] == ".latch")
            {

                if(words[0] == ".latch" && (words.back() == "0" || words.back() == "1" || words.back() == "2" || words.back() == "3"))
                    words.pop_back();   // strip the state off the latches as we don't care

                words.erase(words.begin()); // remove the .names or .latch

                string key = words.back();
                words.pop_back();   // strip the node's own name from the list of connections
                nodeMap[key].inputs = words;   // insert the node in the map
                nodeMap[key].name = key;

                // search in "outputs" for key
                if(std::find(outputs.begin(), outputs.end(), key) != outputs.end())
                    nodeMap[key].inOut = 2; // mark node as output if listed in outputs



                for(int i = 0; i < words.size(); ++i)
                {
                    if(std::find(inputs.begin(), inputs.end(), words[i]) != inputs.end())
                    {
                        nodeMap[key].inOut = 1; // mark node as input if any of its inputs are listed as global inputs
                        // remove global input from list of inputs to the node
                        nodeMap[key].inputs.erase(std::remove(nodeMap[key].inputs.begin(),nodeMap[key].inputs.end(), words[i] ), nodeMap[key].inputs.end());
                    }

                }
                // search words against "inputs"


            }
            else if(words[0] == ".inputs"){
                words.erase(words.begin()); // remove the label
                inputs = words;
            }                                   //POPULATE INPUT/OUTPUT LISTS THESE MUST BE FIRST IN THE FILE
            else if(words[0] == ".outputs"){
                words.erase(words.begin()); // remove the label
                outputs = words;
            }
        }
        myfile.close();
    }
    else
    {
        nodeMap.clear();
        std::cout << "FBB-Partitioner: Unable to read file" << endl;
        return nodeMap;
    }

    // at this point each node only knows its inputs, but has no information about outputs (only half connected)
    // so iterate through the map again, updating the connection lists of each node
    // start with the "in" list of each raw node, visit each "in" node and add our node to it's list of
    // connections if it isn't there already

    std::map<std::string, node>::iterator it = nodeMap.begin();
    struct node workingNode;
    while (it != nodeMap.end())
    {
        workingNode = it->second;

        // this is the ugliest code I've ever written, but it works
        for(int i = 0; i <workingNode.inputs.size(); ++i)
            if(std::find(inputs.begin(), inputs.end(), workingNode.inputs[i]) == inputs.end()) // make sure not to add input nodes
                if (std::find(nodeMap[workingNode.inputs[i]].inputs.begin(),
                              nodeMap[workingNode.inputs[i]].inputs.end(), workingNode.name) ==
                    nodeMap[workingNode.inputs[i]].inputs.end())
                        nodeMap[workingNode.inputs[i]].outputs.push_back(workingNode.name);

        it++;// Increment the Iterator to point to next entry
    }
    return nodeMap;
}