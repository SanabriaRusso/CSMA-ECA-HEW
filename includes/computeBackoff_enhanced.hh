#include "concatenate.hh"
#include "isThisNewBackoffPossible_copy.hh"
#define AC 4

using namespace std;

void computeBackoff_enhanced(std::array<int,AC> &backlog, FIFO <Packet> &Queue, int &category, int &stickiness, std::array<int,AC> &stages, 
	std::array<double,AC> &counters, int &system_stickiness, int &id, int &sx, int &ECA, std::map<double,double> &buffer, 
	std::array<double,AC> &AIFS, const int defaultAIFS[AC])
{

	//CWmin values extracted from Perahia & Stacey's: Next Generation Wireless LANs (p. 240)
	//In number of slots
	int CWmin [AC] = { 32, 32, 16, 8 }; //slots

	double deterministicBackoff;
	double basicDetBackoff;
	int isItPossible;

	bool debug = false;

	deterministicBackoff = (int) (pow(2,(stages.at(category))) * CWmin[category]/2 - 1);
	basicDetBackoff = (int) (pow(2,0) * CWmin[category]/2 - 1);
	AIFS.at(category) = defaultAIFS[category];

	//Always checking if the counters will cause a VC.
	if(ECA == 3)
	{
		isItPossible = isThisNewBackoffPossible_copy(basicDetBackoff, stages, counters, category, backlog, CWmin);	
	}else
	{
		isItPossible = isThisNewBackoffPossible_copy(deterministicBackoff, stages, counters, category, backlog, CWmin);	
	}

	if (debug)
		cout << "Node-" << id << ", AC-" << category << ": is it possible (" << deterministicBackoff << "): " << isItPossible << endl;

	//If it is just a deterministic backoff, we don't have to compute the SmartBackoff
	if(backlog.at(category) == 1)
	{
		if(isItPossible == 1)
		{
			if(sx == 1)
			{
				if(ECA == 1)
				{
					counters.at(category) = deterministicBackoff;

					// cout << "**Node " << id << endl;
					if (debug)
						cout << "\tDeterministic backoff: " << deterministicBackoff << " AIFS " << AIFS.at(category) << endl;
					return; //get out
				}else if(ECA == 3) //Basic ECA
				{
					counters.at(category) = basicDetBackoff;					
				}
			}else
			{
				if(stickiness > 0)
				{
					counters.at(category) = deterministicBackoff;	
					// cout << "**Node " << id << endl;
					if (debug)
						cout << "\tDeterministic backoff: " << deterministicBackoff << " AIFS " << AIFS.at(category) << endl;
					return;
				}
			}
		}
	}else
	{
		stages.at(category) = 0;
		counters.at(category) = 0;
		stickiness = system_stickiness;
		if (debug)
			cout << "\tAC-" << category << " has an empty queue" << endl;
		return;
	}
	


	///////////////////////////
	//////SmartBackoff/////////
	///////////////////////////


	//Looking for an appropriate random backoff in the buffer
	//to avoid repeating the computation everytime.

	double randomBackoff;
	std::array<int, AC> futureCycles;
	std::array<int, AC> compareBackoffs;
	std::array<int, AC> compareCycles; 
	std::array<int, AC> match;
	std::map<double,double>::iterator it;

	match.fill(0);
	futureCycles.fill(1);
	compareBackoffs.fill(1);
	compareCycles.fill(1);


	while ( (compareBackoffs != match) || (compareCycles != match) )
	{
		randomBackoff = rand() % (int) ( (pow(2,stages.at(category))) * CWmin[category] - 1) + 1;
		
		if (debug)
		{
			cout << "\t***Random backoff AC-" << category << ": " << randomBackoff << endl;
		}

		//Avoiding internal collisions with the randomBackoff
		for (int i = AC - 1; i >= 0; i--)
		{
			//Checking if the randomBackoff will collide with successful ACs
			if(i != category)
			{
				if(backlog.at(i) == 1)
				{
					int difference = fabs( counters.at(i) - randomBackoff );
					int othersDetBackoff = (pow(2,stages.at(i)) * CWmin[i]/2);
					int minimum = std::min( (int)othersDetBackoff, (int)deterministicBackoff+1 );
					futureCycles.at(i) = difference % minimum; 
				}else
				{
					futureCycles.at(i) = 1;
				}
			}
		}

		//Filling arrays to make a decision over the chosen random backoff
		for(int j = AC-1; j >= 0; j--)
		{
			if(randomBackoff == counters.at(j))
			{
				compareBackoffs.at(j) = 1;	
			}else
			{
				compareBackoffs.at(j) = 0;
			}

			if(futureCycles.at(j) == 0)
			{
				compareCycles.at(j) = 1;
			}else
			{
				compareCycles.at(j) = 0;
			}

			if (debug)
			{
				if (compareBackoffs.at(j) == 0)
				{
					cout << "\t\t- It is not equal to AC-" << j << "'s backoff." << endl;
				}else
				{
					cout << "\t\t- It is the same as AC-" << j << endl;
				}

				if (compareCycles.at(j) == 0)
				{
					cout << "\t\t- Won't alter collision free operation with AC-" << j << endl;
				}else
				{
					cout << "\t\t- Will collide with AC-" << j << " future sx transmissions" << endl;
				}
			}

		}
	}
	counters.at(category) = randomBackoff;
}