#define AC 4
// #include "isThisNewBackoffPossible.hh"
using namespace std;

void analiseBetterReset(std::array<double,AC> &consecutiveSx, std::array<double,AC> &resetCounters,
	std::array<int,AC> &stages, std::array<double,AC> &counters, int acToTx, const int MAXSTAGES[AC],
	std::array<int,AC> backlog, std::array<int,AC> &resetAttempt, SLOT_notification slot, std::array<int, AC> &shouldReset,
	std::array<int,AC> &resetThresholds, int node, std::array<int,AC> &changeStage, std::array<double,AC> &reset,
	std::array<int, AC> &stationStickiness, int systemStickiness, std::array<double,AC> &analysisCounter, 
	double timer, std::bitset< 512 > &scheduleMap, std::array<int,AC> &resetSx){

	int CWmin [AC] = { 16, 32, 16, 8 }; //slots
	int newStage = 0;
	int halving = 1;	//Just halving (not reseting) the schedule in each attempt
	int dynamicStickiness = 1;	//Incresing the stickiness to a fixed value each time the schedule is modified
	int conservativeApproach = 0;	//Determines which threshold unleashes the analysis of the scheduleMap


	for(int i = 0; i < AC; i++)
	{
		// Can I start registering the slot.status?
		if(resetAttempt.at(i) == 1)
		{
			// Do I still have to do it?
			if(resetCounters.at(i) > 0)
			{
				// Are there any tx in the slot?
				resetCounters.at(i)--;	
				// cout << resetCounters.at(i) << endl;
				if(slot.status > 0)
				{
					// Obtaining the decimal number that will be converted to bitmap in scheduleMap
					shouldReset.at(i) += pow(2, (int)( analysisCounter.at(i) - (resetCounters.at(i) + 1) ) );
					//*/DEBUG
					// cout << "**Node " << node << " tx in slot #" << slot.num << " " << shouldReset.at(i) << endl;
				}
				// cout << resetCounters.at(i) << " " << counters.at(i) << " " <<  slot.num  << " " << slot.status <<  endl;

			}
		}
	}

	//Can we reset the cycle?
	for(int i = 0; i < AC; i++)
	{
		//Are we attempting to reduce the schedule?
		if(resetAttempt.at(i) == 1)
		{
			// If the resetCounter expired
			if(resetCounters.at(i) == 0)
			{
				// Build a bitmap of size 512 with the result of the analysis
				std::bitset< 512 > pastSchedule ( (int) shouldReset.at(i) );
				//Erasing the value of the observations
				shouldReset.at(i) = 0;

				// OR with the complete schedule map
				//*DEBUG
				// cout << "***Building the bitmap" << endl;
				// cout << "pastSchedule: " << pastSchedule << endl;
				// cout << "scheduleMap before: " << scheduleMap << endl;
				// cout << "scheduleMap after OR: ";
				scheduleMap |= pastSchedule;
				// cout << scheduleMap << endl;

				//Resetting the bitmap for last schedule
				pastSchedule.reset();

				//Determining if we can go ahead an analyse the scheduleMap
				int bitmapIteration = (int)resetThresholds.at(i);
				if(conservativeApproach == 1) bitmapIteration = (int)(pow(2,MAXSTAGES[i]) * CWmin[i] / 2) / 
					(pow(2,stages.at(i)) * CWmin[i] / 2);

				if(consecutiveSx.at(i) >= bitmapIteration)
				{
					// cout << "***Checking the bitmap after " << bitmapIteration << " iterations" << endl;
					// Find a shorter deterministic schedule starting from the first slot analysed
					for(int j = 0; j < stages.at(i); j++)
					{
						int increments = (pow(2,j) * CWmin[i]/2);
						if(halving == 1) increments = (pow(2,max(0,stages.at(i)-1)) * CWmin[i]/2);
						int slotAvailable = 0;
						changeStage.at(i) = 0; 

						// Let's check the scheduleMap for empty slots for the corresponding schedules
						//*/DEBUG
						// Looking at the transmissions bitset
						// cout << "**Node " << node << " the bit set is: ";
						// //The analysis counter counts the slot of this node's transmission
						// // up to the last slot before the next transmission
						// for(int b = 0; b < analysisCounter.at(i); b++)
						// {
						// 	cout << scheduleMap[b];
						// }
						// cout << endl;
						//--------------------


						//*/DEBUG
						// cout << "**Node " << node << " checking bitmap position ";
						for(int k = increments; k < analysisCounter.at(i); k += increments)
						{
							slotAvailable += scheduleMap[k];
							//*/DEBUG
							// cout << k << ": " << scheduleMap[k] << "; ";
						}
						//*/DEBUG
						// cout << endl;

						// cout << "Counting from " << increments << " to " << analysisCounter.at(i) -1 << endl;


						// If we found the shorter schedule, stop looking.
						if(slotAvailable == 0)
						{
							changeStage.at(i) = 1;
							newStage = j;
							if(halving == 1) newStage = stages.at(i) -1;
							break;
						}else
						{
							// */DEBUG
							// cout << "--(" << timer <<")Node " << node << endl;
							// cout << "\tTried with stage: " << j << ". Staying in stage " << stages.at(i) << endl;
						}
					}
				}
			}
		}
	}

	//Checking if it is time to schedule a reset of the cycle
	for(int i = 0; i < AC; i++)
	{
		if(backlog.at(i) == 1)
		{
			if(counters.at(i) == 0)
			{	
				// Can we reduce the future cycle?
				if(changeStage.at(i) == 1)
				{
					double newDeterministicBackoff = (int) (pow(2,newStage) * CWmin[i]/2 - 1);
					int canWeChange = isThisNewBackoffPossible(newDeterministicBackoff, stages, counters, i, backlog, CWmin);
					if(canWeChange == 1)
					{
						//Resisting a colision because we changed to another schedule
						if(dynamicStickiness == 1) stationStickiness.at(i) = systemStickiness+1;
						//Reseting the control variables
						resetAttempt.at(i) = 0;
						resetCounters.at(i) = 0;
						consecutiveSx.at(i) = 0;
						changeStage.at(i) = 0;
						reset.at(i)++;
						resetSx.at(i) = 1;
						// */DEBUG
						// cout << "**(" << timer << ")Node " << node << endl;
						// cout << "***Making the change from stage " << stages.at(i);
						// cout << " to " << newStage << " now" << endl;
						stages.at(i) = newStage;
						scheduleMap.reset();
						// cout << "Next analysis to ocurr in slot " << (int)slot.num + (int)newDeterministicBackoff + 2 << endl;
					}else
					{
						//*/DEBUG
						// cout << "The change causes a virtual collisions. Aborting." << endl;
					}

				}

				// ---------------------------
				//Here is where the whole procedure begins
				if(stages.at(i) >= 0)
				{
					//1. Checking the complete schedule
					// resetThresholds.at(i) = ( (int)(pow(2, MAXSTAGES[i]) * CWmin[i] / 2) ) / (int)( pow(2, stages.at(i)) * CWmin[i] / 2 - 1 );

					//2. Checking half of the complete schedule (more aggresive)
					// resetThresholds.at(i) = ( (int)(pow(2, (MAXSTAGES[i]-1) ) * CWmin[i] / 2) / (int)(pow(2, stages.at(i)) * CWmin[i] / 2) ) -1;

					//3. Super aggressive
					resetThresholds.at(i) = 1;

					if(resetThresholds.at(i) <= 0) resetThresholds.at(i) = 1;

					//*/DEBUG
						// cout << "**Node  " << node << endl;
						// cout << "\tNew threshold: " << resetThresholds.at(i) << endl;

					//If you have transmitted successfully for a whole complete cycle, you can starting the analysis
					if( ((int)consecutiveSx.at(i) >= (int)resetThresholds.at(i)) )
					{
						resetAttempt.at(i) = 1;
						// We will look at all the slots between transmissions
						resetCounters.at(i) = (pow(2,stages.at(i)) * CWmin[i]/2);
						// Keeping a record of the selected reset counter 	
						analysisCounter.at(i) = resetCounters.at(i);	
						
						///*DEBUG
						// cout << "**Node " << node << " AC " << i << ": halveCounter: " << resetCounters.at(i) << endl;
						// cout << "\tOwn transmition in the next slot, number: " << (pow(2, stages.at(i)) * CWmin[i] / 2) << "." << endl;
					}
				}
			}
		}
	}
}