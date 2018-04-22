//
// Created by Alan on 01/03/2018.
//
#include <iostream>
#include <fstream>
#include <map>
#include <sstream>
#include <vector>
#include <algorithm>
#include "Hmain.h"

using namespace std;


int hyperToFlow(std::map<std::string, node> &nodeMap)
{
    // at each node
    // process only if prime
    // add two non-prime nodes with appropiate edges
    // search for old (PRIME) nodes we linked to, insert links in them back to us (non prime new node)
    // Check we're not creating new input nodes

    std::map<std::string, node>::iterator it = nodeMap.begin();
    struct node workingNode;
    while (it != nodeMap.end())
    {
        workingNode = it->second;

        if(workingNode.prime)
        {
            //std::cout << "DEBUG/hyperToFlow: I see a prime node, I'm adding a new node called: " << workingNode.name + "N1" << std::endl;

            struct node N2;
            struct node N1;
            if(workingNode.inOut != 2) // don't create new nodes for an output node
            {

                //create new N1 and N2 nodes for the prime node above

                N2.name = workingNode.name + "N2";
                N2.prime = false;
                N2.nState = 2;
                N2.outputs = workingNode.outputs;
                N2.outputs.push_back(workingNode.name);
                N2.outputs_capacity.resize(N2.outputs.size());
                N2.outputs_flow.resize(N2.outputs.size());
                std::fill(N2.outputs_capacity.begin(), N2.outputs_capacity.end(), INT_MAX);
                std::fill(N2.outputs_flow.begin(), N2.outputs_flow.end(), 0);
                N2.relativePrime = workingNode.name;

                N1.name = workingNode.name + "N1";
                N1.prime = false;
                N1.nState = 1;
                N1.outputs.push_back(workingNode.name + "N2");
                N1.outputs_capacity.resize(N1.outputs.size());
                N1.outputs_flow.resize(N1.outputs.size());
                std::fill(N1.outputs_capacity.begin(), N1.outputs_capacity.end(), 1);
                std::fill(N1.outputs_flow.begin(), N1.outputs_flow.end(), 0);
                N1.relativePrime = workingNode.name;

                N1.relativeN2 = N2.name;
                N2.relativeN1 = N1.name;

                nodeMap[N2.name] = N2;
                nodeMap[N1.name] = N1;  // push the new nodes into the map



            }
            // copy old inputs into new outputs with +"N1"
            nodeMap[workingNode.name].outputs.clear();
            for(int i = 0; i < nodeMap[workingNode.name].inputs.size(); i++)
            {
                nodeMap[workingNode.name].outputs.push_back(nodeMap[workingNode.name].inputs[i] + "N1");
            }

            if(N1.name.size() > 0)
                nodeMap[workingNode.name].outputs.push_back(N1.name);

            nodeMap[workingNode.name].inputs.clear();   // we discard these as no longer useful or needed

            //setup the outputs correctly
            nodeMap[workingNode.name].outputs_flow.clear();
            nodeMap[workingNode.name].outputs_capacity.clear();
            nodeMap[workingNode.name].outputs_flow.resize(nodeMap[workingNode.name].outputs.size());
            nodeMap[workingNode.name].outputs_capacity.resize(nodeMap[workingNode.name].outputs.size());
            for(int i = 0; i < nodeMap[workingNode.name].outputs.size(); ++i)
            {
                nodeMap[workingNode.name].outputs_flow[i] = 0;
                nodeMap[workingNode.name].outputs_capacity[i] = INT_MAX;
            }

            nodeMap[workingNode.name].relativeN1 = N1.name;
            nodeMap[workingNode.name].relativeN2 = N2.name; // setup the realtive N nodes
        }

        // Increment the Iterator to point to next entry
        it++;
    }

    return 0;   // could insert an error message in place here
}