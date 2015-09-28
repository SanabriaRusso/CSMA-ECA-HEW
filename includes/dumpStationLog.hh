#define AC 4
#include <iostream>
#include <fstream>
#include <string>

using namespace std;

void dumpStationLog(int id, double &overallThroughput, double &totalCollisions, double &totalHalved)
{
	string logName = "sta-";
	string staNum = to_string(id);
	string end = ".log";
	logName = logName + staNum + end;
	string routeToFile = "../Results/stations/";
	routeToFile = routeToFile + logName;

	ofstream staFile;
	staFile.open(routeToFile.c_str(), ios::app);
	staFile << "#1throughoput, 2 collisions, 3 totalHalved" << endl;
	staFile << overallThroughput << " " << totalCollisions << " " << totalHalved << endl;
	staFile.close();
}