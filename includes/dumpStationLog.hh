#include <iostream>
#include <fstream>
#include <string>

#define AC 4

using namespace std;

void dumpStationLog(int nodes, int id, int prot, ofstream &staFile, double overallThroughput, 
	double fractCollisions, double totalHalved, std::array<int,AC> &finalStages)
{
	int whichAC = 0;

	staFile << "#1nodes, #2 id, #3 prot ,#4 throughoput, #5 collisions, #6 totalHalved, #7 final stage" << endl;
	staFile << nodes << " " << id << " " << prot << " " << overallThroughput << " " 
	<< fractCollisions << " " << totalHalved << " " << finalStages.at(whichAC) << endl;
}