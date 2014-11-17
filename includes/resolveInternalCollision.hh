#include <math.h>
#include <algorithm>

#define AC 4
#define MAXSTAGE 5

int resolveInternalCollision(std::array<int,AC> &backlogg, std::array<double,AC> &qSizes, std::array<int,AC> &stickiness, std::array<int,AC> &stages, std::array<double,AC> &counters, int system_stickiness){

	int iterator = counters.size() - 1;
	int acToTx = -1;
	int internalCollision = 0;

	for (auto rIterator = counters.rbegin(); rIterator < counters.rend(); rIterator++) //a reverse iteration over the arrays
	{
		//Checks if an AC is backlogged AND its backoff counter expired
		//Because it's going backwards, it just need to find the firts (highes priority) one.
		if((backlogg.at(iterator) == 1) && (*rIterator == 0))
		{
			acToTx = iterator;
			break;
		}
		iterator--;
	}


	//Checks if there are any other ACs with an expired backoff. If so, they are treated as internal collisions
	for(int i = 0; i < iterator; i++)
	{
		if((backlogg.at(i) == 1) && (counters.at(i) == 0))
		{
			//cout << "Colliding with " << acToTx << ": " << i << " with counter: " << counters.at(i) << " and stage: " << stages.at(i);
			stickiness.at(i) = std::max((int) stickiness.at(i) - 1, 0);
            stages.at(i) = std::min((int)stages.at(i) + 1, MAXSTAGE);
			computeBackoff(backlogg.at(i), qSizes.at(i), i, stickiness.at(i), stages.at(i), counters.at(i), system_stickiness);
			//cout << ". New counter for AC " << i << ": " << counters.at(i) << ", new stage: " << stages.at(i) << endl;
			internalCollision = 1;
		}
	}

	//if((acToTx >= 0) && (internalCollision == 0)) cout << "Transmitting AC " << acToTx << " witout internal collisions" <<endl;

	return(acToTx);
	
}