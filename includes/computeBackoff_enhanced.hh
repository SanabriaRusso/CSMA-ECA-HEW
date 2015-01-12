#define AC 4

using namespace std;

void computeBackoff_enhanced(int &backlog, FIFO <Packet> &Queue, int &category, int &stickiness, std::array<int,AC> &stages, 
	std::array<double,AC> &counters, int &system_stickiness, int &id, int &sx, int &ECA){

	//CWmin values extracted from Perahia & Stacey's: Next Generation Wireless LANs (p. 240)
	// int CWmin [4] = { 32, 32, 16, 8 };

	int CWmin [4] = { 64, 64, 32, 16 };

	double deterministicBackoff;
	double randomBackoff;
	std::array<int, AC> futureCycles;
	std::array<int, AC> compareBackoffs;
	std::array<int, AC> compareCycles; 
	std::array<int, AC> match;

	deterministicBackoff = (int) (pow(2,(stages.at(category))) * CWmin[category]/2 - 1);
	match.fill(0);
	futureCycles.fill(1);
	compareBackoffs.fill(1);
	compareBackoffs.fill(1);

	while ( (compareBackoffs != match) || (compareCycles != match) )
	{
		randomBackoff = rand() % (int) ( (pow(2,stages.at(category))) * CWmin[category]);
		if(randomBackoff == 0) randomBackoff++;

		//Avoiding internal collisions with the randomBackoff
		for(int i = 0; i < AC; i++)
		{
			//Checking if the randomBackoff will collide with successful ACs
			if(i != category)
			{
				int difference = fabs( (pow(2,stages.at(i)) * CWmin[i]/2) - randomBackoff);
				int minimum = std::min( (pow(2,stages.at(i)) * CWmin[i]/2), randomBackoff );
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