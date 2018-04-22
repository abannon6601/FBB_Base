//
// Created by Alan on 24/02/2018.
//

#ifndef YWPARTITIONPROCESSOR_HMAIN_H
#define YWPARTITIONPROCESSOR_HMAIN_H

#include <vector>
#include <map>
#include <climits>
using namespace std;

// node object
struct node {

    //NODE PROPERTIES
    string name;    // name of the output in the BLIF file (we use as name of the gate)
    int inOut;  // 0:standard node 1:input node 2:output node
    bool prime; // set if node is a prime node or added in flow network
    int nState; // 0 if prime, 1 if N1, 2 if N2

    //NODE DATA
    vector<string> inputs; // inputs to the node    AFTER FLOW GRAPH CONVERSION THIS SHOULD NOT BE USED
    vector<string> outputs; // outputs of the node
    vector<int> outputs_capacity;
    vector<int> outputs_flow;

    //DYNAMIC PROPERTIES
    bool visCurOp;  // during any operation on the matrix this is set high if the node has been visited
    int weight; // as nodes are combined this is increased
    vector<string> mergedNodes; // holds the names of all the nodes we've merged with ONLY USED FOR SOURCE AND SINK NODES
    vector<string> mergedN2Nodes; // holds the names of all the N2 nodes we've merged with ONLY USED FOR SOURCE AND SINK NODES
    bool orphan;    // set true if the original prime node has been merged
    bool marked;            // used to mark if the node is part of a section in cutsize generation

    //NODE RELATIONS
    string relativePrime;
    string relativeN1;
    string relativeN2;

    //HEURISTICS
    int depth;

    //OTHER
    string visitedFrom; // used in BFS for path reporting
    int connectedComponentID;   // used to identify connected components
};

/*// old node object using c++11
 *
 * struct node {

    //NODE PROPERTIES
    string name;    // name of the output in the BLIF file (we use as name of the gate)
    int inOut = 0;  // 0:standard node 1:input node 2:output node
    bool prime = true; // set if node is a prime node or added in flow network
    int nState = 0; // 0 if prime, 1 if N1, 2 if N2

    //NODE DATA
    vector<string> inputs; // inputs to the node    AFTER FLOW GRAPH CONVERSION THIS SHOULD NOT BE USED
    vector<string> outputs; // outputs of the node
    vector<int> outputs_capacity;
    vector<int> outputs_flow;

    //DYNAMIC PROPERTIES
    bool visCurOp = false;  // during any operation on the matrix this is set high if the node has been visited
    int weight = 1; // as nodes are combined this is increased
    vector<string> mergedNodes; // holds the names of all the nodes we've merged with ONLY USED FOR SOURCE AND SINK NODES
    vector<string> mergedN2Nodes; // holds the names of all the N2 nodes we've merged with ONLY USED FOR SOURCE AND SINK NODES
    bool orphan = false;    // set true if the original prime node has been merged
    bool marked = false;            // used to mark if the node is part of a section in cutsize generation

    //NODE RELATIONS
    string relativePrime;
    string relativeN1;
    string relativeN2;

    //HEURISTICS
    int depth = INT_MAX;

    //OTHER
    string visitedFrom; // used in BFS for path reporting
    int connectedComponentID = 0;   // used to identify connected components
};
 */


// FUNCTION DECLARATIONS------------------------------------------------------------------------------------------------
map<string, node> readFile(string fileAdr);
int hyperToFlow(std::map<std::string, node> &nodeMap);

void resetGraph(std::map<std::string, node> &nodeMap);

std::vector<string> findPrimeNeighbors(std::map<std::string, node> &nodeMap, string workingNode);
bool compareByDepth(const node& a,  const node& b);
int update_flow(std::map<std::string, node> &nodeMap, vector<string> path);
void mergeNodes(std::map<std::string, node> &nodeMap,string host, string victim);

std::vector<string> nodesReachableFromX(std::map<std::string, node> &localCircuitGraph, string startNode);
int weightReachableFromX(std::map<std::string, node> &localCircuitGraph, string startNode);
std::vector<string> bfsPathGen(std::map<std::string, node> &localCircuitGraph, string startNode, string targetNode);
std::vector<string> findCutsize(std::map<std::string, node> &localCircuitGraph, string startNode);

int writeToFile(int cutsize, int runtime, float solutionRatio, std::vector<string> cutNets);

int FBB_base(float solutionRatioTarget, float solutionDeviation, string filepath, bool verbose);

int removeConnectedComponents(std::map<std::string, node> &localCircuitGraph);
void initialiseGraph(std::map<std::string, node> &localCircuitGraph);

#endif //YWPARTITIONPROCESSOR_HMAIN_H
