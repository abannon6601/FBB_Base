//
// Created by Alan on 18/04/2018.
//


#include <iostream>
#include <fstream>
#include <map>
#include <sstream>
#include <vector>
#include <algorithm>
#include <list>
#include <queue>
#include <math.h>
#include "Hmain.h"
#include <ctime>
#include <random>

// main function for performaing a flow balanced bipartition
int FBB_base(float solutionRatioTarget, float solutionDeviation, string filepath, bool verbose)
{
    std::cout << "FBB-Partitioner: running" << std::endl;

    std::map<std::string, node> circuitGraph = readFile(filepath);  // read in the hypergraph

    if(circuitGraph.size() < 1)
    {
        std::cout << "FBB-Partitioner: Unable to parse file. " <<  std::endl;
        return 1;
    }

    std::cout << "FBB-Partitioner: Gate nodes in file: " << circuitGraph.size() << std::endl;


    initialiseGraph(circuitGraph);

    int removedNodes = removeConnectedComponents(circuitGraph);
    if(removedNodes > 0)
    {
        std::cout << "FBB-Partitioner: Disconnected components in file! Removing " << removedNodes << " nodes from consideration. This will not affect solution quality"  << std::endl;
    }

    std::clock_t start;
    double duration;

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

    srand(time(NULL));

    int inputIndex = (0 + (rand() % (int)(inputs.size())));
    int outputIndex = (0 + (rand() % (int)(outputs.size())));

    string source   =   inputs[inputIndex];
    string sink     =   outputs[outputIndex];       // TODO EXTENSION: choose these nodes

    // OVERRIDE for sink/source
    /*
    source = "a";
    sink = "h";
    std::cout << "DEBUG: WARNING! SOURCE/SINK OVERRIDEN!"<< std::endl;
    */

    std::cout << "FBB-Partitioner: Source node chosen as: " << source << " Sink node chosen as: " << sink << std::endl;
    std::cout << "FBB-Partitioner: Solution ratio set at: " << (int) 100*solutionRatioTarget << "% Skew tolerance set at: " << (int) 100*solutionDeviation<< "%" << std::endl;

    hyperToFlow(circuitGraph);   // convert the hypergraph to a flow graph

    int nodes = circuitGraph.size(); // update the number of nodes in the graph after flow conversion

    //SetDepthMetric(circuitGraph, sink); // set the depth metric for the flow graph

    std::cout << "FBB-Partitioner: Setup complete. Processing..." << std::endl;

    // START OF LOOP

    //loop variables
    vector<string> foundPath;
    start = std::clock();   // start a clock

    for(int cycles = 0; cycles < nodes; cycles++)  // don't do more loops than nodes (if this is ever needed something has gone wrong, so quit)
    {

        do {

            resetGraph(circuitGraph);
            /*
            std::cout<<"DEBUG: Executing BFS: "<<std::endl;
            start = std::clock();   // start a clock

            duration = ( std::clock() - start ) / (double) CLOCKS_PER_SEC;
            std::cout<<"DEBUG: BFS took: "<< duration <<std::endl;
             */

            foundPath = bfsPathGen(circuitGraph, source, sink);

            if (foundPath.size() < 1)        // we don't have a path
            {
                break;
            }

            std::reverse(foundPath.begin(), foundPath.end());    // flip the path to be from source to sink

            update_flow(circuitGraph, foundPath);    // push more flow along the path
        } while (foundPath.size() > 0);    // repeat until we run out of paths



        //look at the size of the sections we have
        resetGraph(circuitGraph);
        int sourceSideSize = weightReachableFromX(circuitGraph, source) + removedNodes; // include the removed nodes (if any) here
        resetGraph(circuitGraph);
        int sinkSideSize = weightReachableFromX(circuitGraph, sink);

        //std::cout <<"\33[2K\r";  // clear the line  UNRELIABLE BETWEEN CONSOLE ENVIROMENTS
        if(verbose)
            std::cout << "FBB-Partitioner-VERBOSE: current section sizes: Source: " <<sourceSideSize << " Sink: " << sinkSideSize << endl;

        // check to see if the current partition is good enouch
        if (abs((solutionRatioTarget * nodes) - sourceSideSize) < (int) ceil(solutionDeviation * (float) nodes) || abs((solutionRatioTarget * nodes) - sourceSideSize) < (int) ceil(solutionDeviation * (float) nodes)) {
            //if the current division is good, return it
            std::cout << "FBB-Partitioner: Valid solution found, analysing..." << std::endl;
            duration = ( std::clock() - start ) / (double) CLOCKS_PER_SEC;

            resetGraph(circuitGraph);
            vector<string> cutNets = findCutsize(circuitGraph, sink);
            int cutSize = cutNets.size();

            std::cout << std::endl << "FBB-Partitioner: Cutsize: " << cutSize << std::endl;
            std::cout << "FBB-Partitioner: Area ratio: " << (float) sourceSideSize/nodes << std::endl;
            std::cout << "FBB-Partitioner: Runtime: " << duration <<"s" << std::endl;
            std::cout << "FBB-Partitioner: Writing results to file..." << std::endl;

            if(writeToFile(cutSize, duration, (float) sourceSideSize/nodes, cutNets) == 0)
                std::cout << "FBB-Partitioner: Write succeeded." << std::endl;
            else
                std::cout << "FBB-Partitioner: Write failed" << std::endl;



            return 0;
        }

        //if we haven't found an acceptable size list all the reachable nodes the source side
        vector<string> reachableNodes;
        string mergeSide;

        if(sourceSideSize < sinkSideSize)
        {   // merge into source
            resetGraph(circuitGraph);
            reachableNodes = nodesReachableFromX(circuitGraph, source);
            mergeSide = source;
        }
        else
        {   // merge into sink
            resetGraph(circuitGraph);
            reachableNodes = nodesReachableFromX(circuitGraph, sink);
            mergeSide = sink;
        }

        //first strip non-prime and source and sink from the list
        int i = 0;
        while (i < reachableNodes.size())
        {

            if (!(circuitGraph[reachableNodes[i]].prime) || reachableNodes[i] == source || reachableNodes[i] == sink)
            {
                reachableNodes.erase(reachableNodes.begin() + i);
                i = 0;
            }
            else
            {
                ++i;
            }
        }

        //merge all nodes remaining
        for (int i = 0; i < reachableNodes.size(); i++) {

            if(verbose)
                cout<<"FBB-Partitioner-VERBOSE: Merging " << reachableNodes[i] <<" into " << mergeSide << " during a group merge"<< endl;

            mergeNodes(circuitGraph, mergeSide, reachableNodes[i]);
        }

        //merge a neighbor of the new supernode;

        vector<string> neighbors = findPrimeNeighbors(circuitGraph, mergeSide);

        // strip source and sink from the list
        int j = 0;
        while (j < neighbors.size())
        {

            if (neighbors[j] == source || neighbors[j] == sink)
            {
                neighbors.erase(neighbors.begin() + j);
            }
            else
            {
                ++j;
            }
        }

        if (neighbors.size() < 1) {
            std::cout << "FBB-Partitioner: No neighbors found" << std::endl;
            return 1;
        }

        mergeNodes(circuitGraph, mergeSide, neighbors[0]);   //TODO, choose this rather than going for 0-index

        if(verbose)
            cout<<"FBB-Partitioner-VERBOSE: Merging " << neighbors[0] <<" into " << mergeSide << endl;

    }


    std::cout << "FBB-Partitioner: failed to find valid partition. Try again with different parameters" << std::endl;

    return 2;


}

// clears the visCurOp flag for every node in the graph
void resetGraph(std::map<std::string, node> &nodeMap)
{
    std::map<std::string, node>::iterator it = nodeMap.begin();

    while (it != nodeMap.end())
    {
        it->second.visCurOp = false;
        it->second.visitedFrom.clear();
        it++;
    }
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
        if(!nodeMap[temp.outputs[i]].orphan)    // don't list merged nodes
            neighbors.push_back(nodeMap[temp.outputs[i]].relativePrime);
    }

    if(temp.mergedN2Nodes.size() > 0) //if we have merged N2 nodes
    {
        node tempN2;
        for(int i  = 0; i < temp.mergedN2Nodes.size(); i++) // look at all merged N2s
        {
            tempN2 = nodeMap[temp.mergedN2Nodes[i]];

            for(int i = 0; i < tempN2.outputs.size(); i++)  // look at all outputs of each N2
            {
                if(tempN2.outputs[i] != workingNode)
                    neighbors.push_back(tempN2.outputs[i]); // Add them to the neighbors list if they don't link back to ourself
            }
        }
    }


    if(temp.inOut != 2) // After merge there may be one than one relative N2!
    {
        temp = nodeMap[nodeMap[workingNode].relativeN2];    // switch to the original n2 node

        for (int i = 0; i < temp.outputs.size(); ++i)
        {
            if(temp.outputs[i] == workingNode)  // don't include the prime link
                continue;

            neighbors.push_back(temp.outputs[i]);   // these should all be prime nodes
        }
    }

    return neighbors;
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

// resetGraph MUST BE RUN BEFORE THIS
// returns the summed weight of nodes reachable from node X without passing through a saturated edge
int weightReachableFromX(std::map<std::string, node> &localCircuitGraph, string startNode)
{
    queue <string> processQ;
    int result = 0;

    processQ.push(startNode);

    node tempNode;
    vector <string> outputs;

    while(processQ.size() > 0)
    {
        tempNode=localCircuitGraph[processQ.front()];

        if(tempNode.visCurOp)
        {
            processQ.pop();
            continue;       // we've already logged this node
        }
        else
            localCircuitGraph[tempNode.name].visCurOp = true;


        for(int i = 0; i < tempNode.outputs.size(); i++)
        {
            if(tempNode.outputs_flow[i] < tempNode.outputs_capacity[i])
            {
                processQ.push(tempNode.outputs[i]); // queue all outputs not saturated to be processed
            }
        }

        result = result + tempNode.weight;

        processQ.pop();
    }

    return result;

}

std::vector<string> bfsPathGen(std::map<std::string, node> &localCircuitGraph, string startNode, string targetNode)
{
    queue <string> processQ;

    processQ.push(startNode);

    node tempNode;
    vector <string> outputs;

    vector<string> path;




    while(processQ.size() > 0)
    {

        tempNode=localCircuitGraph[processQ.front()];

        if(tempNode.name == targetNode)
            break;

        if(tempNode.visCurOp)
        {
            processQ.pop();
            continue;       // we've already logged this node
        }
        else
            localCircuitGraph[tempNode.name].visCurOp = true;


        for(int i = 0; i < tempNode.outputs.size(); i++)
        {
            if(tempNode.outputs_flow[i] < tempNode.outputs_capacity[i])
            {
                if(localCircuitGraph[tempNode.outputs[i]].visitedFrom.size() < 1)   // only look at outputs that haven't already been proessed
                {
                    processQ.push(tempNode.outputs[i]); // queue all outputs not saturated to be processed
                    localCircuitGraph[tempNode.outputs[i]].visitedFrom = tempNode.name;
                }
            }
        }

        processQ.pop();
    }






    if(localCircuitGraph[targetNode].visitedFrom.size() > 0)    // if we actually reached the target work backwards using visited From
    {


        localCircuitGraph[startNode].visitedFrom.clear(); // shouldn't be needed, but why take chances

        queue <string> processQ2;
        processQ2.push(targetNode);

        path.push_back(targetNode);

        while(processQ2.size() > 0)
        {
            tempNode = localCircuitGraph[processQ2.front()];

            if(tempNode.visitedFrom.size() > 0)
            {
                processQ2.push(tempNode.visitedFrom);
                path.push_back(tempNode.visitedFrom);
            }

            processQ2.pop();
        }

        return path;
    }
    else
    {
        path.clear();
        return path;
    }

}

// resetGraph MUST BE RUN BEFORE THIS
// returns a vector of nodes reachable from node X without passing through a saturated edge
std::vector<string> nodesReachableFromX(std::map<std::string, node> &localCircuitGraph, string startNode)
{
    queue <string> processQ;
    vector <string> resultSet;

    processQ.push(startNode);

    node tempNode;
    vector <string> outputs;

    while(processQ.size() > 0)
    {
        tempNode=localCircuitGraph[processQ.front()];

        if(tempNode.visCurOp)
        {
            processQ.pop();
            continue;       // we've already logged this node
        }

        else
            localCircuitGraph[tempNode.name].visCurOp = true;

        for(int i = 0; i < tempNode.outputs.size(); i++)
        {
            if(tempNode.outputs_flow[i] < tempNode.outputs_capacity[i])
            {
                processQ.push(tempNode.outputs[i]); // queue all outputs not saturated to be processed
            }
        }

        resultSet.push_back(tempNode.name);

        processQ.pop();
    }

    return resultSet;

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

        if(nodeMap[vicitmNode.outputs[i]].outputs.size() < 1)
            continue;

        string workingNode = nodeMap[vicitmNode.outputs[i]].outputs[0];  // this should be the N2 node that links back to us

        // remove the link to the victim
        nodeMap[workingNode].outputs.erase(std::remove( nodeMap[workingNode].outputs.begin(),  nodeMap[workingNode].outputs.end(), victim),  nodeMap[workingNode].outputs.end());

        // change it to link to the host if it doesn't already have one
        if(std::find(nodeMap[workingNode].outputs.begin(), nodeMap[workingNode].outputs.end(), host) == nodeMap[workingNode].outputs.end())
            nodeMap[workingNode].outputs.push_back(host);
    }

    // set strctures after merge;

    hostNode.mergedNodes.push_back(victim); // add the victim to the list of merged nodes
    hostNode.mergedN2Nodes.push_back(vicitmNode.relativeN2);

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

// report the cutsize
std::vector<string> findCutsize(std::map<std::string, node> &localCircuitGraph, string startNode)
{
    /* sequence:
     * mark all reachable nodes
     *
     * go thourgh all reachable nodes and see if a neighbor exists that's unmarked (unreachable)
     * add it to cutnets and mark that neighbor
     *
     */

    queue <string> processQ;
    vector<string> result;

    processQ.push(startNode);

    node tempNode;
    vector <string> outputs;

    // mark all reachable nodes
    while(processQ.size() > 0)
    {
        tempNode=localCircuitGraph[processQ.front()];

        if(tempNode.visCurOp)
        {
            processQ.pop();
            continue;       // we've already logged this node
        }

        else
            localCircuitGraph[tempNode.name].visCurOp = true;

        for(int i = 0; i < tempNode.outputs.size(); i++)
        {
            if(tempNode.outputs_flow[i] < tempNode.outputs_capacity[i])
            {
                processQ.push(tempNode.outputs[i]); // queue all outputs not saturated to be processed
            }
        }

        localCircuitGraph[tempNode.name].marked = true;

        processQ.pop();
    }

    // reset all the other markers
    resetGraph(localCircuitGraph);

    processQ.push(startNode);

    //go thourgh all reachable nodes and see if a neighbor exists that's unmarked (unreachable)
    while(processQ.size() > 0)
    {
        tempNode=localCircuitGraph[processQ.front()];

        if(tempNode.visCurOp)
        {
            processQ.pop();
            continue;       // we've already logged this node
        }

        else
            localCircuitGraph[tempNode.name].visCurOp = true;

        for(int i = 0; i < tempNode.outputs.size(); i++)
        {
            if(tempNode.outputs_flow[i] < tempNode.outputs_capacity[i])
            {
                processQ.push(tempNode.outputs[i]); // queue all outputs not saturated to be processed
            }
            else    // we potentially have a link to an unreachable neighbor on the other side of a cut.
            {
                if(!localCircuitGraph[tempNode.outputs[i]].marked) // we have an unreachable node hasn't already been counted
                {
                    localCircuitGraph[tempNode.outputs[i]].marked = true;
                    result.push_back(localCircuitGraph[tempNode.outputs[i]].relativePrime);   // incriment the cutsize
                }
            }
        }


        processQ.pop();
    }

    return result;
}

// write results to a file
int writeToFile(int cutsize, int runtime, float solutionRatio, std::vector<string> cutNets)
{
    //TODO add a date/time stamp to output file


    ofstream resultFile;
    resultFile.open ("FBB_results.txt");

    if (resultFile.is_open())
    {
        resultFile << "FBB Algorthim results:\n" << "Cutsize: " << cutsize << " Runtime: " << runtime << " Area ratio: " << solutionRatio << endl;
        resultFile << "Cut nets:" << endl << endl;


        for(int i  = 0; i < cutNets.size(); i++)
            resultFile <<cutNets[i]<<endl;

        resultFile.close();
        return 0;
    }
    else
    {
        return 1;
    }
}

// function find all connected components and removes all but the largest, returns number of nodes removed this way
int removeConnectedComponents(std::map<std::string, node> &localCircuitGraph)
{
    std::map<std::string, node>::iterator it = localCircuitGraph.begin();

    int connectedComponents = 0;
    vector<int> connectedComponentSize;
    int removedNodes = 0;

    while (it != localCircuitGraph.end())
    {
        if(it->second.connectedComponentID < 1)    // we have a new connected component
        {
            connectedComponentSize.push_back(0);
            queue <string> processQ;

            // reset the graph because we're about to traverse it
            resetGraph(localCircuitGraph);

            processQ.push(it->second.name);

            node tempNode;
            vector <string> outputs;

            while(processQ.size() > 0)
            {
                tempNode=localCircuitGraph[processQ.front()];

                if(tempNode.visCurOp)
                {
                    processQ.pop();
                    continue;       // we've already logged this node
                }

                else
                    localCircuitGraph[tempNode.name].visCurOp = true;

                for(int i = 0; i < tempNode.outputs.size(); i++)
                {
                    processQ.push(tempNode.outputs[i]); // queue all outputs to be processed
                }
                for(int i = 0; i < tempNode.inputs.size(); i++)
                {
                    processQ.push(tempNode.inputs[i]); // queue all inputs to be processed
                }

                localCircuitGraph[tempNode.name].connectedComponentID = connectedComponents + 1;    //mark the node as being in component (connectedComponents)
                connectedComponentSize[connectedComponents]++;  // increase the size of this component

                processQ.pop();
            }
            connectedComponents++;
        }
        it++;
    }

    if(connectedComponents > 1) // we have multiple connected components
    {
        int N = connectedComponentSize.size() / sizeof(int);
        int indexOfMax = distance(connectedComponentSize.begin(), max_element(connectedComponentSize.begin(), connectedComponentSize.end()));
        int largestComponentID = indexOfMax + 1;

        connectedComponentSize.erase(connectedComponentSize.begin() + indexOfMax);// remove the largest connected component from the vector

        // sum how many nodes are in other components:

        for(std::vector<int>::iterator it =connectedComponentSize.begin(); it != connectedComponentSize.end(); ++it)
            removedNodes += *it;

        // removed and using code above
        //removedNodes = accumulate(connectedComponentSize.begin(), connectedComponentSize.end(), 0);

        cout<<endl;

        // remove all the nodes not marked as part of the largest group
        it = localCircuitGraph.begin();
        while (it != localCircuitGraph.end())
        {
            if(it->second.connectedComponentID != largestComponentID)
                localCircuitGraph.erase(it++);
            else
                ++it;
        }

    }



    return removedNodes;
}

// Function to initialise the graph as the Gatech ECE cluster won't compile C++11 because our sysadmin has the technical competence of a concussed squirrel
// ONLY RUN BEFORE HYPERGRAPH CONVERSION
void initialiseGraph(std::map<std::string, node> &localCircuitGraph)
{
    std::map<std::string, node>::iterator it = localCircuitGraph.begin();

    while (it != localCircuitGraph.end())
    {
        it->second.visCurOp = false;  // during any operation on the matrix this is set high if the node has been visited
        it->second.weight = 1; // as nodes are combined this is increased
        it->second.prime = true; // as nodes are combined this is increased
        it->second.orphan = false;    // set true if the original prime node has been merged
        it->second.marked = false;            // used to mark if the node is part of a section in cutsize generation
        it->second.depth = INT_MAX;
        it->second.connectedComponentID = 0;   // used to identify connected components
        it++;
    }
}





// HEURISTIC SETTING FUNCTIONS:-----------------------------------------------------------------------------------------

// Sets the depth metric of all nodes reachable from startNode.
void SetDepthMetric(std::map<std::string, node> &localCircuitGraph, string startNode)
{
    /*
     * Sequence:
     *
     * start: Push workingNode into a queue:
     * loop:
     *  pull a node from the queue
     *  look at all the depths of PRIME nodes it links to
     *  if they're less than our depth + 1 update them
     *  and add them to the list of nodes to be processed
     *  repeat:
     */

    queue <string> processQ;

    localCircuitGraph[startNode].depth = 0; // setup the depth at the start
    processQ.push(startNode);

    node tempNode;
    int tempDepth;

    while(processQ.size() > 0)
    {
        tempNode = localCircuitGraph[processQ.front()];
        tempDepth = tempNode.depth;

        vector <string> neighbors = findPrimeNeighbors(localCircuitGraph, processQ.front());

        for(int i  = 0; i < neighbors.size(); i ++)
        {

            if(localCircuitGraph[neighbors[i]].depth > (tempDepth + 1))
            {
                localCircuitGraph[neighbors[i]].depth = tempDepth + 1;
                if(localCircuitGraph[neighbors[i]].inOut != 2)
                {
                    localCircuitGraph[localCircuitGraph[neighbors[i]].relativeN1].depth = tempDepth + 1;
                    localCircuitGraph[localCircuitGraph[neighbors[i]].relativeN2].depth = tempDepth + 1;
                }
                processQ.push(neighbors[i]);
            }
        }

        processQ.pop();
    }
}



// OLD FUNCTIONS NO LONGER USED-----------------------------------------------------------------------------------------

// Even though this is not recursive resetGraph MUST BE RUN before using this function.
std::vector<string> bfsGuidedPathGen(std::map<std::string, node> &localCircuitGraph, string startNode, string targetNode)
{
    /*
     * Sequence:
     *
     * start:
     *      Push startNode into the queue
     * loop while we have ndoes in the queue:
     *      Look at its neighbors (both prime and non-prime) that are unsaturated
     *      if any are the target add it to the path and return
     *      if not pick the best neighbor (not fobidden) to the queue and finish
     *      if we can't continue, and haven't reached the target, mark it as illegal (visCurOp = true)
     *      -and add the last node before us back into the queue.
     *
     */

    queue <string> processQ;
    vector <string> path;       // path we return

    processQ.push(startNode);

    node tempNode;
    vector <string> outputs;

    int nodesProcessed = 0;

    while(processQ.size() > 0)
    {
        nodesProcessed++;

        tempNode = localCircuitGraph[processQ.front()];
        outputs.clear();

        // extract all VALID outputs
        for(int i = 0; i < tempNode.outputs.size(); i++)
        {
            if(!localCircuitGraph[tempNode.outputs[i]].visCurOp && (tempNode.outputs_capacity[i] > tempNode.outputs_flow[i]))
            {
                outputs.push_back(tempNode.outputs[i]);

                if(tempNode.outputs[i] == targetNode)   // we reached target
                {
                    path.push_back(tempNode.name);
                    path.push_back(tempNode.outputs[i]);
                    cout<<"DEBUG: BFS search processed " << nodesProcessed << " nodes" <<endl;
                    return path;
                }

            }
        }

        // Check outputs
        if(outputs.size() < 1)  // we don't have any valid paths from this node
        {
            if(path.size() < 1) // we've exhausted all nodes
            {
                path.clear();
                cout<<"DEBUG: BFS search processed " << nodesProcessed << " nodes" <<endl;
                return path;
            }


            if(std::find(path.begin(), path.end(), tempNode.name) != path.end())
            {
                //cout<<"DEBUG: BFS removing from path " << path.back() << endl;
                path.pop_back();
            }


            if(path.size() > 0)
            {
                processQ.push(path.back()); //push the last node back into the queue
                localCircuitGraph[tempNode.name].visCurOp = true; // mark this node as a dead-end
                //cout<<"DEBUG: BFS backtracking to "<< path.back() << endl;
                processQ.pop();
                continue;
            }
            else
            {
                path.clear();
                cout<<"DEBUG: BFS search processed " << nodesProcessed << " nodes" <<endl;
                return path;    // we ran out of nodes
            }



        }


        // set up an array of nodes so we can sort the nieghbors by depth
        vector<node> neighborsNodes;
        for(int i = 0; i < outputs.size(); i++)
        {
            neighborsNodes.push_back(localCircuitGraph[outputs[i]]);
        }

        // sort the neighbors by depth
        std::sort(neighborsNodes.begin(), neighborsNodes.end(), compareByDepth);

        processQ.push(neighborsNodes[0].name); // select the node with the lowest depth to process next
        localCircuitGraph[tempNode.name].visCurOp = true;
        processQ.pop();

        if(path.size() > 0) // path.back segfaults if called on empty vector
        {
            if (path.back() != tempNode.name)    // the name may already be there if we backtracked
            {
                path.push_back(tempNode.name);
            }
        }
        else
        {
            path.push_back(tempNode.name);
        }

        //DEBUG SECTION

        if(processQ.size() > 10) // something is fucky
            cout<<"DEBUG: BFS search process queue is overflowing" <<endl;

        /*
        if(path.size() > localCircuitGraph.size()/10)
        {
            cout<<"DEBUG: BFS path is off-scale high" <<endl;
            path.clear();
            return path;
        }
         */




    }

    // at this point we've failed to find a path
    path.clear();
    cout<<"DEBUG: BFS search processed " << nodesProcessed << " nodes" <<endl;
    return path;


}