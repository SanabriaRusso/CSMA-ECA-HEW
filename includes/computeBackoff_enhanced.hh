#include "concatenate.hh"
#include "isThisNewBackoffPossible_copy.hh"
#define AC 4

using namespace std;

void computeBackoff_enhanced(std::array<int,AC> &backlog, FIFO <Packet> &Queue, int &category, int &stickiness, std::array<int,AC> &stages, 
	std::array<double,AC> &counters, int &system_stickiness, int &id, int &sx, int &ECA, std::map<double,double> &buffer, 
	std::array<double,AC> &AIFS, const int defaultAIFS[AC]){

	//CWmin values extracted from Perahia & Stacey's: Next Generation Wireless LANs (p. 240)
	//In number of slots
	int CWmin [AC] = { 16, 32, 16, 8 }; //slots

	double deterministicBackoff;
	double basicDetBackoff;
	int isItPossible;

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
					// cout << "\tDeterministic backoff: " << deterministicBackoff << " AIFS " << AIFS.at(category) << endl;
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
					// cout << "\tDeterministic backoff: " << deterministicBackoff << " AIFS " << AIFS.at(category) << endl;
					return;
				}
			}
		}
	}else
	{
		stages.at(category) = 0;
		counters.at(category) = 0;
		stickiness = system_stickiness;
		// cout << "\tAC " << category << " has an empty queue" << endl;
		return;
	}
	


	///////////////////////////
	//////SmartBackoff/////////
	///////////////////////////


	//Looking for an appropriate random backoff in the buffer
	//to avoid repeating the computation everytime.

	// std::map<double,double>::iterator it;

	// unsigned hash1 = concatenate(counters.at(0), counters.at(1));
	// // cout << "hash1: " << hash1;
	// unsigned hash2 = concatenate(counters.at(2), counters.at(3));
	// // cout << ". hash2: " << hash2;
	// unsigned hash = concatenate(hash1, hash2);
	// // cout << ". hash: " << hash;
	// hash = concatenate(hash, (unsigned)stages.at(category));
	// // cout << ". final: " << hash << endl;
	// it = buffer.find(hash);

	// if(it == buffer.end())	//If hash is not in buffer
	// {
		// cout << "Not in buffer: " << hash << endl;

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
			randomBackoff = rand() % (int) ( (pow(2,stages.at(category))) * CWmin[category] - 1);

			//Avoiding internal collisions with the randomBackoff
			for(int i = 0; i < AC; i++)
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
			for(int i = 0; i < AC; i++)
			{
				if(randomBackoff == counters.at(i))
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
			// Debug info
			// if( (compareBackoffs != match) || (compareCycles != match) )
			// {
			// 	cout << "---AC " << category << " Failed to compute SmartBackoff with: " << randomBackoff << " AIFS " << 
			// 		defaultAIFS[category] << endl;
			// 	for(int i = 0; i < AC; i++)
			// 	{
			// 		cout << "\nAC " << i << ": " << counters.at(i) << "\tAIFS " <<  AIFS.at(i) << endl;
			// 	}
			// 	cout << endl << endl;
			// }
		}
		
		counters.at(category) = randomBackoff;
		// buffer[hash] = randomBackoff;
		// cout << "Adding it: " << buffer[hash] << endl;

		// Debug info
		// cout << "+++SmartBackoff with: " << randomBackoff << " AIFS " << defaultAIFS[category] << endl;
		// for(int i = 0; i < AC; i++)
		// {
		// 	cout << "\nAC " << i << ": " << counters.at(i) << " \tAIFS " << AIFS.at(i) << " ";
		// }
		// cout << endl << endl;


	// }else
	// {
	// 	counters.at(category) = it->second;	//second value pointed by the iterator. That is, the value.
	// 	// cout << "Buffered [" << it->first << "]: " << it->second << endl;
	// }
}