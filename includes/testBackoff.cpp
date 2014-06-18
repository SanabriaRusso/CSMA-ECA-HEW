#include <iostream>
#include <cstdlib>
#include <string>
#include <sstream>
#include <fstream>
#include "backoff.hh"

using namespace std;

int main (int argc, const char * argv[])
{
	int backoffCounter = 0;
	int backoffStage = 0;
	int stickiness = 0;
	float slotDrift = 0;
	
	cout << "Please, enter the backoff stage: " << endl;
	cin >> backoffStage;
	cout << "Station stickiness?: " << endl;
	cin >> stickiness;
	cout << "Slot drift probability [0,1]: " << endl;
	cin >> slotDrift;
		
	backoffCounter = backoff(backoffStage, stickiness, slotDrift);
	
	cout << "The appropriate backoff counter is: " << backoffCounter << endl;
}