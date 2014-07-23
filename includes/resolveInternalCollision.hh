#include <math.h>
#include <algorithm>

#define AC 4
#define MAXSTAGE 5

using namespace std;

int resolveInternalCollision(std::array<int,AC> &backlogg, std::array<double,AC> &qSizes, std::array<int,AC> &stickiness, std::array<int,AC> &stages, std::array<double,AC> &counters){

	int iterator = counters.size() - 1;
	int acToTx = 0;

	for (auto rIterator = counters.rbegin(); rIterator < counters.rend(); rIterator++)
	{
		if(*rIterator == 0)
		{
			acToTx = iterator;
			break;
		}
		iterator--;
	}

	for(int i = 0; i < iterator; i++)
	{
		if(counters.at(i) == 0)
		{
			//cout << "Colliding AC: " << i << " counter: " << counters.at(i);
			stickiness.at(i) = std::max((int) stickiness.at(i) - 1, 0);
            stages.at(i) = std::min((int)stages.at(i) + 1, MAXSTAGE);
			computeBackoff(backlogg.at(i), qSizes.at(i), i, stickiness.at(i), stages.at(i), counters.at(i));
			//cout << ". New counter: " << counters.at(i) << endl;

		}
	}

	return(acToTx);
	
}