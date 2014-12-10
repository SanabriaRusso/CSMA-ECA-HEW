#include <math.h>
#include <algorithm>

#define AC 4
#define MAXSTAGE 5

int resolveInternalCollision(std::array<int,AC> &backlogg, std::array<FIFO <Packet>, AC> &Queues, 
	std::array<int,AC> &stickiness, std::array<int,AC> &stages, std::array<double,AC> &counters, 
	int system_stickiness, int id, std::array<double,AC> &totalInternalACCol, std::array<int,AC> &retAttemptAC,
	double simTime, int ECA,std::array<int,AC> &recomputeBackoff){

	int iterator = counters.size() - 1;
	int acToTx = -1;

	for (auto rIterator = counters.rbegin(); rIterator < counters.rend(); rIterator++) //a reverse iteration over the arrays
	{
		//Checks if an AC is backlogged AND its backoff counter expired
		//Because it's going backwards, it just need to find the firts (highes priority) one.
		if((backlogg.at(iterator) == 1) && (*rIterator == 0))
		{
			acToTx = iterator;
			//cout << "(" << simTime << ") STA-" << id << ": AC: " << iterator << " backlogg: " << backlogg.at(iterator) << endl;
			break;
		}
		iterator--;
	}


	//Checks if there are any other ACs with an expired backoff. If so, they are treated as internal collisions
	if(iterator >= 0)
	{
		for(int i = 0; i < iterator; i++)
		{
			if((backlogg.at(i) == 1) && (counters.at(i) == 0))
			{
				//cout << "(" << simTime << ") STA-" << id << ": Colliding internally with " << acToTx << ": " << i 
				//	<< " with counter: " << counters.at(i) << " and stage: " << stages.at(i) << endl;

				stickiness.at(i) = std::max((int) stickiness.at(i) - 1, 0);
	            stages.at(i) = std::min((int)stages.at(i) + 1, MAXSTAGE);
	            //cout << "STA-" << id << ": internalCollision: " << acToTx << " and " << i << endl;
				
				recomputeBackoff.at(i) = 1;

				// computeBackoff(backlogg.at(i), Queues.at(i), i, stickiness.at(i), stages.at(i), 
					// counters.at(i), system_stickiness, id, sx, ECA);
				//cout << ". New counter for AC " << i << ": " << counters.at(i) << ", new stage: " << stages.at(i) << endl;
				totalInternalACCol.at(i)++;

				retAttemptAC.at(i)++;
			}else
			{
				recomputeBackoff.at(i) = 0;
			}
		}
	}

	return(acToTx);
	
}