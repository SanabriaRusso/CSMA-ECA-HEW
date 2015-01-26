#include "concatenate.hh"
#define AC 4

using namespace std;

void computeBackoff_enhanced(std::array<int,AC> &backlog, FIFO <Packet> &Queue, int &category, 
	int &stickiness, std::array<int,AC> &stages, std::array<double,AC> &counters, int &system_stickiness, 
	int &id, int &sx, int &ECA, std::map<unsigned long long int,double> &buffer, int forceRandom){

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
	std::map<unsigned long long int,double>::iterator it;

	deterministicBackoff = (int) (pow(2,(stages.at(category))) * CWmin[category]/2 - 1);


	//Assigning the backoff to the correspondent AC
	if(forceRandom == 0)
	{
		if(backlog.at(category) == 1)
		{
			// cout << "Node " << id << ". AC " << category << " Old counter: " << counters.at(category) << endl;
			if(sx == 1)
			{
				if(ECA == 1)
				{
					counters.at(category) = deterministicBackoff;
					// cout << "+++Node " << id << " AC " << category << " ECA: " << counters.at(category) << endl;
					return;
				}
			}else
			{
				if(stickiness > 0)
				{
					counters.at(category) = deterministicBackoff;
					// cout << "+++Node " << id << " AC " << category << " ECA (hyst): " << counters.at(category) << endl;	
					return;			
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
	}

	match.fill(0);
	futureCycles.fill(1);
	compareBackoffs.fill(1);
	compareCycles.fill(1);


	//Looking for an appropriate random backoff in the buffer
	//to avoid repeating the computation everytime.

	unsigned long long int hash1 = concatenate(counters.at(0), counters.at(1));
	cout << "hash1: " << hash1;
	unsigned long long int hash2 = concatenate(counters.at(2), counters.at(3));
	cout << ". hash2: " << hash2 << endl;
	unsigned long long int hash = concatenate(hash1, hash2);
	cout << "hash: " << hash << endl;
	unsigned long long int stages1 = concatenate(stages.at(0), stages.at(1));
	cout << "stages1: " << stages1;
	unsigned long long int stages2 = concatenate(stages.at(2), stages.at(3));
	cout <<". stages2: " << stages2 << endl;
	unsigned long long int stagesFinal = concatenate(stages1, stages2);
	cout << "stages final: " << stagesFinal << endl;

	unsigned long long int hashFinal = concatenate(hash, stagesFinal);
	cout << "final: " << hashFinal << endl;

	it = buffer.find(hashFinal);

	if(it == buffer.end())	//If hash is not in buffer
	{
		cout << "Not in buffer: " << hash << endl;

		//Keeping track of the backoff generated for certain hash
		std::map<double,int> tried;
		std::map<double,int>::iterator it2;
		while ( (compareBackoffs != match) || (compareCycles != match) )
		{
			cout << "Calculating for ac: " << category << endl;
			cout << "AC 0: " << counters.at(0) << ". AC 1: " << counters.at(1);
			cout << ". AC 2: " << counters.at(2) << ". AC 3: " << counters.at(3) << endl; 

			bool used = false;
			while(!used)
			{
				randomBackoff = rand() % (int) ( (pow(2,stages.at(category))) * CWmin[category] - 1);

				it2 = tried.find(randomBackoff);
				if(it2 == tried.end())
				{
					used = true;
					tried[randomBackoff] = 1;
				}

			}
			
			cout << "Random backoff to test: " << randomBackoff << endl;

			//Avoiding internal collisions with the randomBackoff
			for(int i = 0; i < AC; i++)
			{
				//Checking if the randomBackoff will collide with successful ACs
				if( (i != category) && (backlog.at(i) > 0) )
				{
					int halfCWmin = (int) CWmin[i]/2;
					int othersDetBackoff = (pow(2,stages.at(i)) * halfCWmin) -1;
					int difference = fabs( counters.at(i) - randomBackoff );
					int minimum = std::min( (int)othersDetBackoff, (int)deterministicBackoff );

					cout << "minimum: " << minimum << ". difference: " << difference << endl;

					futureCycles.at(i) = difference % minimum; 

					if(futureCycles.at(i) != 0)
					{
						cout << "It works" << endl;
					}else
					{
						cout << "It doesn't work" << endl;
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
		}
		buffer[hashFinal] = randomBackoff;
		cout << "******Adding it: " << buffer[hashFinal] << endl;
	}else
	{
		randomBackoff = it->second;	//second value pointed by the iterator. That is, the value.
		cout << "Buffered [" << it->first << "]: " << it->second << endl;
	}

	if(sx == 1)
	{
		if(ECA == 0)
		{
			counters.at(category) = randomBackoff;
				// cout << "---Node " << id << " AC " << category << " DCF: " << counters.at(category) << endl;
		}
	}else
	{
		if((stickiness == 0) || (forceRandom ==1) )
		{
			counters.at(category) = randomBackoff;
				// cout << "---Node " << id << " AC " << category << " DCF (col): " << counters.at(category) << endl;
		}
			
	}

}