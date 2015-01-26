#include "concatenate.hh"
#define AC 4

using namespace std;

void computeBackoff_enhanced(int &backlog, FIFO <Packet> &Queue, int &category, int &stickiness, std::array<int,AC> &stages, 
	std::array<double,AC> &counters, int &system_stickiness, int &id, int &sx, int &ECA, std::map<double,double> &buffer){

	//CWmin values extracted from Perahia & Stacey's: Next Generation Wireless LANs (p. 240)
	int CWmin [4] = { 32, 32, 16, 8 };

	// int CWmin [4] = { 64, 64, 32, 16 };
	// int CWmin [4] = { 1024, 1024, 1024, 1024 };

	double deterministicBackoff;
	double randomBackoff;
	std::array<int, AC> futureCycles;
	std::array<int, AC> compareBackoffs;
	std::array<int, AC> compareCycles; 
	std::array<int, AC> match;
	std::map<double,double>::iterator it;

	deterministicBackoff = (int) (pow(2,(stages.at(category))) * CWmin[category]/2 - 1);
	match.fill(0);
	futureCycles.fill(1);
	compareBackoffs.fill(1);
	compareBackoffs.fill(1);


	//Looking for an appropriate random backoff in the buffer
	//to avoid repeating the computation everytime.

	unsigned hash1 = concatenate(counters.at(0), counters.at(1));
	// cout << "hash1: " << hash1;
	unsigned hash2 = concatenate(counters.at(2), counters.at(3));
	// cout << ". hash2: " << hash2;
	unsigned hash = concatenate(hash1, hash2);
	// cout << ". hash: " << hash;
	hash = concatenate(hash, (unsigned)stages.at(category));
	// cout << ". final: " << hash << endl;
	it = buffer.find(hash);

	if(it == buffer.end())	//If hash is not in buffer
	{
		// cout << "Not in buffer: " << hash << endl;
		while ( (compareBackoffs != match) || (compareCycles != match) )
		{
			randomBackoff = rand() % (int) ( (pow(2,stages.at(category))) * CWmin[category] - 1);
			if(randomBackoff == 0) randomBackoff++;

			//Avoiding internal collisions with the randomBackoff
			for(int i = 0; i < AC; i++)
			{
				//Checking if the randomBackoff will collide with successful ACs
				if(i != category)
				{
					int difference = fabs( (pow(2,stages.at(i)) * CWmin[i]/2 -1) - randomBackoff);
					int minimum = std::min( (pow(2,stages.at(i)) * CWmin[i]/2 -1), deterministicBackoff );
					futureCycles.at(i) = difference % minimum; 
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
		}
		buffer[hash] = randomBackoff;
		// cout << "Adding it: " << buffer[hash] << endl;
	}else
	{
		randomBackoff = it->second;	//second value pointed by the iterator. That is, the value.
		// cout << "Buffered [" << it->first << "]: " << it->second << endl;
	}

	//Assigning the backoff to the correspondent AC

	if(backlog == 1)
	{
		// cout << "Node " << id << ". AC " << category << " Old counter: " << counters.at(category) << endl;
		if(sx == 1)
		{
			if(ECA == 1)
			{
				counters.at(category) = deterministicBackoff;
				// cout << "+++Node " << id << " AC " << category << " ECA: " << counters.at(category) << endl;
			}else
			{
				counters.at(category) = randomBackoff;
				// cout << "---Node " << id << " AC " << category << " DCF: " << counters.at(category) << endl;
			}
		}else
		{
			if(stickiness > 0)
			{
				counters.at(category) = deterministicBackoff;
				// cout << "+++Node " << id << " AC " << category << " ECA (hyst): " << counters.at(category) << endl;				
			}else
			{
				counters.at(category) = randomBackoff;
				// cout << "---Node " << id << " AC " << category << " DCF (col): " << counters.at(category) << endl;
			}
			
		}
	}else
	{
		stages.at(category) = 0;
		counters.at(category) = 0;
		stickiness = system_stickiness;
		// cout << "\tAC " << category << " has an empty queue" << endl;
	}
}