#include "concatenate.hh"
#define AC 4

using namespace std;

void computeBackoff_enhanced(std::array<int,AC> &backlog, FIFO <Packet> &Queue, int &category, int &stickiness, std::array<int,AC> &stages, 
	std::array<double,AC> &counters, int &system_stickiness, int &id, int &sx, int &ECA, std::map<double,double> &buffer, 
	std::array<double,AC> &AIFS){

	//CWmin values extracted from Perahia & Stacey's: Next Generation Wireless LANs (p. 240)
	//In number of slots
	int CWmin [AC] = { 32, 32, 16, 8 };

	// int CWmin [AC] = { 64, 64, 32, 16 };
	// int CWmin [AC] = { 1024, 1024, 1024, 1024 };

	//Default AIFS extracted from Perahia & Stacey's: Next Generation Wireless LANs (p. 240)
	//In number of slots
	// int defaultAIFS [AC] = { 8, 4, 2, 1 };
	int defaultAIFS [AC] = { 0, 0, 0, 0 };

	double deterministicBackoff;

	deterministicBackoff = (int) (pow(2,(stages.at(category))) * (CWmin[category]/2) - 1); //check decrement.hh
	AIFS.at(category) = defaultAIFS[category];

	//If it is just a deterministic backoff, we don't have to compute the SmartBackoff
	if(backlog.at(category) == 1)
	{
		if(sx == 1)
		{
			if(ECA == 1)
			{
				counters.at(category) = deterministicBackoff;
				cout << "+++Deterministic backoff: " << deterministicBackoff << " AIFS " << AIFS.at(category) << endl;
				return; //get out
			}

		}else
		{
			if(stickiness > 0)
			{
				counters.at(category) = deterministicBackoff;	
				cout << "+++Deterministic backoff: " << deterministicBackoff << " AIFS " << AIFS.at(category) << endl;
				return;
			}
		}

	}else
	{
		stages.at(category) = 0;
		counters.at(category) = 0;
		stickiness = system_stickiness;
		cout << "\tAC " << category << " has an empty queue" << endl;
		return;
	}
	


	///////////////////////////
	//////SmartBackoff/////////
	///////////////////////////

	double randomBackoff;
	std::array<double,AC> otherCounters;
	std::array<double,AC> othersDetBackoff;
	std::array<int, AC> futureCycles;
	std::array<int, AC> compareBackoffs;
	std::array<int, AC> compareCycles; 
	std::array<int, AC> match;
	std::map<double,double>::iterator it;

	match.fill(0);
	futureCycles.fill(1);
	compareBackoffs.fill(1);
	compareCycles.fill(1);


	//Filling an array with the number of slots until the next transmissions
	for(int i = 0; i < AC; i++)
	{
		otherCounters.at(i) = counters.at(i) + AIFS.at(i); // in number of slots - 1
		othersDetBackoff.at(i) = pow(2,stages.at(i)) * (CWmin[i]/2) + defaultAIFS[i]; // in number of slots
	}

	while ( (compareBackoffs != match) || (compareCycles != match) )
	{
		randomBackoff = rand() % (int) ( pow(2,stages.at(category)) * CWmin[category] ) - 1;

		//Avoiding internal collisions with the randomBackoff
		for(int i = 0; i < AC; i++)
		{
			//Checking if the randomBackoff will collide with successful ACs
			if(i != category)
			{
				if(backlog.at(i) == 1)
				{
					int difference = fabs( otherCounters.at(i) - (randomBackoff + defaultAIFS[category]) ); // in number of slots
					// int othersDetBackoff = (pow(2,stages.at(i)) * CWmin[i]/2) + defaultAIFS[i];
					int minimum = std::min( (int)othersDetBackoff.at(i), (int)othersDetBackoff.at(category) );
					futureCycles.at(i) = difference % minimum; 
				}else
				{
					futureCycles.at(i) = 1;
				}
			}
		}

		//Filling arrays to make a decision over the chosen random backoff
		for(int i = 0; i < AC; i++)
		{
			if( ( (randomBackoff + defaultAIFS[category]) == otherCounters.at(i) )
				|| (randomBackoff == counters.at(i)) )
			{
				compareBackoffs.at(i) = 1;	
			}else
			{
				compareBackoffs.at(i) = 0;
			}

			if(futureCycles.at(i) == 0)
			{
				compareCycles.at(i) = 1;
			}else
			{
				compareCycles.at(i) = 0;
			}

		}

			//Debug info
			if( (compareBackoffs != match) || (compareCycles != match) )
			{
				cout << "---AC " << category << " Failed to compute SmartBackoff with: " << randomBackoff << " AIFS " << 
					defaultAIFS[category] << endl;
				for(int i = 0; i < AC; i++)
				{
					cout << "\nAC " << i << ": " << counters.at(i) << "\tAIFS " <<  AIFS.at(i) << endl;
				}
				cout << endl << endl;
			}
		}
	
		//Debug info
		cout << "+++SmartBackoff with: " << randomBackoff << " AIFS " << defaultAIFS[category] << endl;
		counters.at(category) = randomBackoff;
		for(int i = 0; i < AC; i++)
		{
			cout << "\nAC " << i << ": " << counters.at(i) << " \tAIFS " << AIFS.at(i) << " ";
		}
		cout << endl << endl;
}