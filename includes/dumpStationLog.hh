#include <iostream>
#include <fstream>
#include <string>

using namespace std;

void dumpStationLog(int nodes, int id, ofstream &staFile, double overallThroughput, double fractCollisions, double totalHalved)
{
	staFile << "#1nodes, #2 id, #3 throughoput, #4 collisions, #5 totalHalved" << endl;
	staFile << nodes << " " << id << " " << overallThroughput << " " << fractCollisions << " " << totalHalved << endl;
}