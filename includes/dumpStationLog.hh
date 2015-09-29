#include <iostream>
#include <fstream>
#include <string>

using namespace std;

void dumpStationLog(int nodes, int id, int prot, ofstream &staFile, double overallThroughput, 
	double fractCollisions, double totalHalved)
{
	staFile << "#1nodes, #2 id, #3 prot ,#4 throughoput, #5 collisions, #6 totalHalved" << endl;
	staFile << nodes << " " << id << " " << prot << " " << overallThroughput << " " 
	<< fractCollisions << " " << totalHalved << endl;
}