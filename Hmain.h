//
// Created by Alan on 24/02/2018.
//

#ifndef YWPARTITIONPROCESSOR_HMAIN_H
#define YWPARTITIONPROCESSOR_HMAIN_H

#include <vector>
#include <map>
#include <climits>
using namespace std;

// simple node object
struct node {
    string name;    // name of the output in the BLIF file (we use as name of the gate)
    int inOut = 0;  // 0:standard node 1:input node 2:output node
    bool prime = true; // set if node is a prime node or added in flow network
    int nState = 0; // 0 if prime, 1 if N1, 2 if N2
    vector<string> inputs; // inputs to the node    AFTER FLOW GRAPH CONVERSION THIS SHOULD NOT BE USED
    vector<string> outputs; // outputs of the node
    vector<int> outputs_capacity;
    vector<int> outputs_flow;
    bool visCurOp = false;  // during any operation on the matrix this is set high if the node has been visited
    int weight = 1; // as nodes are combined this is increased

    vector<string> mergedNodes; // holds the names of all the nodes we've merged with ONLY USED FOR SOURCE AND SINK NODES
    vector<string> mergedN2Nodes; // holds the names of all the N2 nodes we've merged with ONLY USED FOR SOURCE AND SINK NODES
    string relativePrime;
    string relativeN1;
    string relativeN2;
    bool orphan = false;    // set true if the original prime node has been merged

    int depth = INT_MAX;

    string visitedFrom; // used in BFS for path reporting
};


map<string, node> readFile(string fileAdr);
int hyperToFlow(std::map<std::string, node> &nodeMap);
void resetGraph(std::map<std::string, node> &nodeMap);
std::vector<string> bfs_path_generate(std::map<std::string, node> &nodeMap, string workingNode, string targetNode);
int update_flow(std::map<std::string, node> &nodeMap, vector<string> path);
bool bfs_path_exists(std::map<std::string, node> &nodeMap, string workingNode, string sink, vector<string> illegalNodes);
std::vector<string> findNeighbors(std::map<std::string, node> &nodeMap, string workingNode, string source, string sink);

#endif //YWPARTITIONPROCESSOR_HMAIN_H
