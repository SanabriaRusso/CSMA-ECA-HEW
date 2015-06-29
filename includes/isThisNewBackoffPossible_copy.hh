#define AC 4
using namespace std;

int isThisNewBackoffPossible_copy(double test, std::array<int,AC> &stages, std::array<double,AC> &counters, int &category,
	std::array<int,AC> &backlog, int CWmin[AC]){
	
	//The code below is an extract of the one found in computeBackoff_enhanced.hh

	int isItPossible = 0;
	std::array<int, AC> futureCycles;
	std::array<int, AC> compareBackoffs;
	std::array<int, AC> compareCycles; 
	std::array<int, AC> match;
	std::map<double,double>::iterator it;

	match.fill(0);
	futureCycles.fill(1);
	compareBackoffs.fill(1);
	compareCycles.fill(1);

	int deterministicBackoff = (int) (pow(2,(stages.at(category))) * CWmin[category]/2 - 1);

	
	for(int i = 0; i < AC; i++)
	{
		//Checking if the test will collide with successful ACs
		if(i != category)
		{
			if(backlog.at(i) == 1)
			{
				int difference = fabs( counters.at(i) - test );
				int othersDetBackoff = (pow(2,stages.at(i)) * CWmin[i]/2);
				int minimum = std::min( (int)othersDetBackoff, (int)deterministicBackoff+1 );
				futureCycles.at(i) = difference % minimum; 
			}else
			{
				futureCycles.at(i) = 1;
			}
		}
	}

	//Filling arrays to make a decision over the test
	for(int i = 0; i < AC; i++)
	{
		if(test == counters.at(i))
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


	if ( (compareBackoffs == match) && (compareCycles == match) )
	{
		isItPossible = 1;
	}else
	{
		isItPossible = 0;
	}

	return(isItPossible);

}
