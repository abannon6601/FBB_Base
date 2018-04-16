/*
 * Current Issues:
 *
 * non-prime nodes not being removed from potential merge set
 *
 * need to choose whether to merge all source or sink side nodes: at the moment just defaulting to source-side
 *
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
#include <math.h>
#include "Hmain.h"

using namespace std;


void resetVisCurOp(std::map<std::string, node> &nodeMap);
void setDepthMetric(std::map<std::string, node> &nodeMap, string workingNode, int depth);
std::vector<string> findPrimeNeighbors(std::map<std::string, node> &nodeMap, string workingNode);
std::vector<string> bfs_path_generate(std::map<std::string, node> &nodeMap, string workingNode, string targetNode);
bool compareByDepth(const node& a,  const node& b);
int update_flow(std::map<std::string, node> &nodeMap, vector<string> path);
void reachableNodesFromX(std::map<std::string, node> &nodeMap, string workingNode);
int sizeReachableFromX(std::map<std::string, node> &nodeMap, string workingNode);
void mergeNodes(std::map<std::string, node> &nodeMap,string host, string victim);

float solutionRatioTarget = 0.5f;
float solutionDeviation = 0.1f;              // TODO: both of these should be passed as arguments

std::vector<string> reachableGLOBAL;    // used to store reachable nodes from another node



int main() {

    std::cout << "FBB-Partitioner: running" << std::endl;

    std::map<std::string, node> circuitGraph = readFile("C:/Users/Alan/CLionProjects/FBB_partitioner/benchmark_files/testModel.blif");  // read in the hypergraph

    std::cout << "FBB-Partitioner: Gate nodes in file: " << circuitGraph.size() << std::endl;

    std::vector<string> inputs;
    std::vector<string> outputs;
    auto it = circuitGraph.begin();
    // populate list of inputs and outputs
    while (it != circuitGraph.end())
    {
        if(it->second.inOut == 1)   // add to list of inputs
            inputs.push_back(it->second.name);
        if(it->second.inOut == 2)   // add to list of outputs
            outputs.push_back(it->second.name);
        it++;
    }

    string source = "a";    // TODO THESE ARE SET FOR THE TEST MODEL ONLY CHANGE FOR ANY OTHER MODEL
    string sink = "i";       // TODO EXTENSION: choose these nodes

    hyperToFlow(circuitGraph);   // convert the hypergraph to a flow graph

    int nodes = circuitGraph.size(); // update the number of nodes in the graph after flow conversion

    std::cout << "FBB-Partitioner: Setup complete. Processing" << std::endl;


    vector<string> foundPath;

    circuitGraph[sink].depth = 1;    // setup the depth of the sink to be 1

    resetVisCurOp(circuitGraph);
    setDepthMetric(circuitGraph, sink, 0); // set the depth metric for the flow graph

    // START OF LOOP

    for(int cycles = 0; cycles < nodes; cycles++)  // don't do more loops than nodes
    {

        do {
            resetVisCurOp(circuitGraph);
            foundPath = bfs_path_generate(circuitGraph, source, sink);

            if (foundPath.size() < 1)        // we don't have a path
            {
                continue;
            }
            if (foundPath.front() != sink)   // we didn't reach the sink, so no augmenting path
            {
                continue;
            }

            std::reverse(foundPath.begin(), foundPath.end());    // flip the path to be from source to sink

            update_flow(circuitGraph, foundPath);    // push more flow along the path
        } while (foundPath.size() > 0);    // repeat until we run out of paths






        //look at the size of the sections we have
        resetVisCurOp(circuitGraph);
        int sourceSideSize = sizeReachableFromX(circuitGraph, source);
        resetVisCurOp(circuitGraph);
        int sinkSideSize = sizeReachableFromX(circuitGraph, sink);

        // check to see if the current partition is good enouch
        if (abs((solutionRatioTarget * nodes) - sourceSideSize) < (int) ceil(solutionDeviation * (float) nodes)) {
            //if the current division is good, return it
            //TODO handle this
            std::cout << "FBB-Partitioner: VALID PARTITION FOUND" << std::endl;
            return 0;
        }





        //if we haven't found an acceptable size list all the reachable nodes the source side
        vector<string> reachableNodes;

        if(sourceSideSize < sinkSideSize)
        {   // merge into source
            resetVisCurOp(circuitGraph);
            reachableNodesFromX(circuitGraph, source);
            reachableNodes = reachableGLOBAL;
        }
        else
        {   // merge into sink
            resetVisCurOp(circuitGraph);
            reachableNodesFromX(circuitGraph, sink);
            reachableNodes = reachableGLOBAL;
        }

        reachableGLOBAL.clear();



        //first strip non-prime and source and sink from the list
        for (int i = 0; i < reachableNodes.size(); i++)
        {
            if(!circuitGraph[reachableNodes[i]].prime)
                std::cout<<"I SEE A NON_PRIME NODE" <<std::endl;

            if (!(circuitGraph[reachableNodes[i]].prime) || reachableNodes[i] == source || reachableNodes[i] == sink)
            {
                reachableNodes.erase(reachableNodes.begin() + i);
                int i = 0;
            }
        }

        std::cout  << "This is just a debug stopper" << std::endl;



        /*
         *

        //first strip none-prime and the source itself from the list
        for (int i = 0; i < reachableNodes.size(); i++)
        {
            if (!(circuitGraph[reachableNodes[i]].prime) || reachableNodes[i] == source)
            {
                reachableNodes.erase(reachableNodes.begin() + i);
            }
        }

        //merge all nodes remaining
        for (int i = 0; i < reachableNodes.size(); i++) {
            mergeNodes(circuitGraph, source, reachableNodes[i]);
        }

        //merge a neighbor (or reachable?) of the new supernode;

        vector<string> neighbors = findPrimeNeighbors(circuitGraph, source);

        if (neighbors.size() < 1) {
            std::cout << "FBB-Partitioner: No neighbors found" << std::endl;
            return 1;
        }

        mergeNodes(circuitGraph, source, neighbors[0]);   //TODO, choose this rather than going for 0-index

                  *
         */


    }


    //repeat and try to add more flow


    std::cout << "FBB-Partitioner: failed to find valid partition. Try again with different parameters" << std::endl;

    return 1;
}



// clears the visCurOp flag for every node in the graph
void resetVisCurOp(std::map<std::string, node> &nodeMap)
{
    std::map<std::string, node>::iterator it = nodeMap.begin();

    while (it != nodeMap.end())
    {
        it->second.visCurOp = false;
        it++;
    }
}

void setDepthMetric(std::map<std::string, node> &nodeMap, string workingNode, int depth)
{
    if (nodeMap.count(workingNode) == 0) // called on non-existant node
    {
        //cout<< "setDepthMetric CALLED ON NON-EXISTENT NODE" <<endl;
        return;
    }

    node temp;
    temp = nodeMap[workingNode];

    depth++;

    // we go prime node to prime nodes ONLY
    vector <string> neighbors = findPrimeNeighbors(nodeMap, workingNode);


    if(temp.depth > depth)
    {
        nodeMap[workingNode].depth = depth; // we found a shorter path to the node, so update the flow and keep going
        if(nodeMap[workingNode].inOut != 2)
        {
            nodeMap[nodeMap[workingNode].relativeN1].depth = depth;
            nodeMap[nodeMap[workingNode].relativeN2].depth = depth;
        }
        for(int i = 0; i < neighbors.size(); i++)
            setDepthMetric(nodeMap, neighbors[i], depth);
        nodeMap[workingNode].visCurOp = true;
        return;
    }

    if (temp.visCurOp)   // we've already pinged this node
        return;
    else
        nodeMap[workingNode].visCurOp = true;

    for(int i = 0; i < neighbors.size(); i++)
        setDepthMetric(nodeMap, neighbors[i], depth);   // we may not have updated the flow, but we haven't visited this node (not sure this will ever happen)

}

// finds all prime node neighbors of a prime node (DO NOT CALL ON N-NODES)
std::vector<string> findPrimeNeighbors(std::map<std::string, node> &nodeMap, string workingNode)
{
    node temp;
    temp = nodeMap[workingNode];
    vector<string> neighbors;

    // add all the prime outputs to the list of neighbors
    for (int i = 0; i < temp.outputs.size(); ++i)
    {
        if(temp.outputs[i] == temp.relativeN1)  // don't include the n1 link
            continue;

        // this will point to N1 nodes, so go look for the primes
        neighbors.push_back(nodeMap[temp.outputs[i]].relativePrime);
    }

    if(temp.inOut != 2)
    {
        temp = nodeMap[temp.relativeN2];    // switch to the n2 node (need to check this existss)
        for (int i = 0; i < temp.outputs.size(); ++i)
        {
            if(temp.outputs[i] == workingNode)  // don't include the prime link
                continue;

            neighbors.push_back(temp.outputs[i]);   // these should all be prime nodes
        }
    }

    return neighbors;
}

std::vector<string> bfs_path_generate(std::map<std::string, node> &nodeMap, string workingNode, string targetNode)
{
    if(nodeMap.count(workingNode) == 0) // called on non-existant node
    {
        //cout<< "BFS CALLED ON NON-EXISTENT NODE" <<endl;
        return{};
    }

    node temp;
    temp = nodeMap[workingNode];

    if(temp.visCurOp)   // we've already pinged this node
        return{};
    else
        nodeMap[workingNode].visCurOp = true;

    vector<string> result;

    if(temp.name == targetNode) // we've been called on the target
    {
        result.push_back(workingNode);
        return result;                  // return our node up the recursion
    }

    else
    {
        vector<string> neighborsNames;

        // add only unsaturated nodes
        for(int i = 0; i < temp.outputs.size(); i++)
        {
            if(temp.outputs_capacity[i] > temp.outputs_flow[i])
                neighborsNames.push_back(temp.outputs[i]);
        }

        if(neighborsNames.size() < 1)
            return {};  // we don't have any unsaturated links;


        vector<node> neighborsNodes;
        for(int i = 0; i < neighborsNames.size(); i++)
        {
            neighborsNodes.push_back(nodeMap[neighborsNames[i]]);
        }

        // sort the neighbors by depth
        std::sort(neighborsNodes.begin(), neighborsNodes.end(), compareByDepth);


        // this loop only ends if we have a path to the target, or we run out of neighbors;
        bool foundTarget = false;
        for(int i = 0; i < neighborsNodes.size() && !foundTarget; i++)
        {
            result = bfs_path_generate(nodeMap, neighborsNodes[i].name, targetNode);
            if(result.size() > 0)
                if(result.front() == targetNode)
                    foundTarget = true;             // this arrangment is clumsy, but needed to avoid a segfault
        }

        if(foundTarget)
        {
            result.push_back(workingNode);
            return result;
        }

    }

    return {};

}

// simple function used to sort a vector of nodes
bool compareByDepth(const node& a,  const node& b)
{
    return a.depth < b.depth;
}

int update_flow(std::map<std::string, node> &nodeMap, vector<string> path)
{
    if(path.size() < 1)
        return 0;

    // Lemma 1: the amaount of flow on a single path through the network is always 1

    // start with infinite flow (INT_MAX)
    // take the first element
    // work out the index of the output from the node name
    // put as much flow as possible into that connection (to capacity)
    // update the amount of flow we can carry down (equal to the max amout we could put on that connection)
    // go to the next node and repeat

    std::vector<string>::iterator it;
    int targetIndex;
    node workingNode;

    for(int i = 0; i <path.size() - 1; ++i)
    {
        workingNode = nodeMap[path[i]];
        it = std::find(workingNode.outputs.begin(), workingNode.outputs.end(), path[i+1]);  // find the index of the next node
        targetIndex = it - workingNode.outputs.begin(); // targetIndex now holds the index of the output of the workingNode along the path

        if(workingNode.outputs_flow[targetIndex] < workingNode.outputs_capacity[targetIndex])   // if we have space for more flow
        {
            nodeMap[workingNode.name].outputs_flow[targetIndex] += 1;
        }
        else
        {
            throw("FLow update called on path with saturated node");    // if this happens something is seriously wrong. Abort execution.
        }
    }


    return 1;
}

int sizeReachableFromX(std::map<std::string, node> &nodeMap, string workingNode)
{
    if(nodeMap.count(workingNode) == 0) // called on non-existant node
        return 0;

    node temp;
    temp = nodeMap[workingNode];

    if(temp.visCurOp)   // we've already pinged this node
        return 0;
    else
        nodeMap[workingNode].visCurOp = true;

    int result = 0;

    result += temp.weight;

    for(int i = 0; i < temp.outputs.size(); i++)
    {
        if(temp.outputs_flow[i] < temp.outputs_capacity[i])
        {
            result += sizeReachableFromX(nodeMap, temp.outputs[i]);
        }

    }
    return result;
}

// fills the global reachableGLOBAL vector with nodes reachble from workingNode
void reachableNodesFromX(std::map<std::string, node> &nodeMap, string workingNode)
{
    if(nodeMap.count(workingNode) == 0) // called on non-existant node
        return;

    node temp;
    temp = nodeMap[workingNode];

    if(temp.visCurOp)   // we've already pinged this node
        return;
    else
        nodeMap[workingNode].visCurOp = true;

    reachableGLOBAL.push_back(workingNode);

    for(int i = 0; i < temp.outputs.size(); i++)
    {
        if(temp.outputs_flow[i] < temp.outputs_capacity[i])
        {
            reachableNodesFromX(nodeMap, temp.outputs[i]);
        }

    }
    return;

}

// merges victim into host. Eliminates victim with extreme prejudice
void mergeNodes(std::map<std::string, node> &nodeMap,string host, string victim)
{
    // host should consume all inputs and outputs of the victim WITHOUT DUPLICATION

    // copying outputs is easy, inputs can be found with a loop through the N1 nodes to the N2 nodes that link back to us

    node hostNode = nodeMap[host];
    node vicitmNode = nodeMap[victim];

    //  copy outputs of the victim node
    for(int i = 0; i < vicitmNode.outputs.size(); ++i)
        if(std::find(hostNode.outputs.begin(), hostNode.outputs.end(), vicitmNode.outputs[i]) == hostNode.outputs.end()) // check to insure we don't get duplications
        {
            hostNode.outputs.push_back(vicitmNode.outputs[i]);
            hostNode.outputs_capacity.push_back((vicitmNode.outputs_capacity[i]));  // copy the outputs WITH FLOW AND CAPACITY
            hostNode.outputs_flow.push_back((vicitmNode.outputs_flow[i]));
        }




    // look at all the outputs of the victim (N1 nodes)
    for(int i = 0; i < vicitmNode.outputs.size(); ++i)
    {
        // for each output go to the N2 node

        string workingNode = nodeMap[vicitmNode.outputs[i]].outputs[0];  // this should be the N2 node that links back to us

        // remove the link to the victim
        nodeMap[workingNode].outputs.erase(std::remove( nodeMap[workingNode].outputs.begin(),  nodeMap[workingNode].outputs.end(), victim),  nodeMap[workingNode].outputs.end());

        // change it to link to the host if it doesn't already have one
        if(std::find(nodeMap[workingNode].outputs.begin(), nodeMap[workingNode].outputs.end(), host) == nodeMap[workingNode].outputs.end())
            nodeMap[workingNode].outputs.push_back(host);
    }

    // set strctures after merge;

    hostNode.mergedNodes.push_back(victim); // add the victim to the list of merged nodes

    hostNode.weight += vicitmNode.weight;

    if(nodeMap[victim].inOut != 2)
    {
        nodeMap[nodeMap[victim].relativeN1].orphan = true;
        nodeMap[nodeMap[victim].relativeN2].orphan = true;  // orphan the non-prime nodes
    }


    // push and kill
    nodeMap[host] = hostNode;
    nodeMap.erase(victim);
}

