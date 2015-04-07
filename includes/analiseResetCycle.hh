#define AC 4

using namespace std;

void analiseResetCycle(std::array<double,AC> &consecutiveSx, std::array<double,AC> &resetCounters,
	std::array<int,AC> &stages, std::array<double,AC> &counters, int acToTx, const int MAXSTAGES[AC],
	std::array<int,AC> backlog, std::array<int,AC> &resetAttempt, SLOT_notification slot, std::array<int, AC> &shouldReset,
	std::array<int,AC> &resetThresholds, int node, std::array<int,AC> &changeStage, std::array<double,AC> &reset,
	std::array<int, AC> &stationStickiness, int systemStickiness, std::array<double,AC> &analysisCounter){

	int CWmin [AC] = { 32, 32, 16, 8 }; //slots
	int newStage = 0;


	for(int i = 0; i < AC; i++)
	{
		// Can I start registering the slot.status?
		if(resetAttempt.at(i) == 1)
		{
			// Do I still have to do it?
			if(resetCounters.at(i) >= 0)
			{
				// Are there any tx in the slot?
				if(slot.status > 0)
				{
					shouldReset.at(i) += pow(2, (int)( analysisCounter.at(i) - resetCounters.at(i) ) );
				}
				// cout << "**Node " << node << " saving slot state (" << slot << ")" << endl;
				resetCounters.at(i)--;	
			}
			//*/DEBUG
			// if(resetCounters.at(i) != -1) 
			// 	cout << "Node " << node << " slot #" << resetCounters.at(i) << ", slot status: " <<  s << endl;
		}
	}

	//Can we reset the cycle?
	for(int i = 0; i < AC; i++)
	{
		// If the resetCounter expired
		if(resetCounters.at(i) == -1)
		{
			// Build a bitmap of size 512 with the result of the analysis
			std::bitset< 512 > scheduleMap ( (int) shouldReset.at(i) );
			
			// Find a shorter deterministic schedule starting from the first slot analysed
			for(int j = 0; j < stages.at(i); j++)
			{
				int increments = (pow(2,j) * CWmin[i]/2);
				int slotAvailable = 0;
				changeStage.at(i) = 0; 
				// Let's check the scheduleMap for empty slots for the corresponding schedules
				// Notice that the last analysisCounter.at(i) + 1 will always be zero
				// The last analysisCounter.at(i) + 1 is the slot before the node's next tx
				for(int k = increments; k <= analysisCounter.at(i) + 1; k += increments)
				{
					slotAvailable += scheduleMap[k];
				}
				// If we found the shorter schedule, stop looking.
				if(slotAvailable == 0)
				{
					changeStage.at(i) = 1;
					newStage = j;
					break;
				}
			}
		}
	}

	//Checking if it is time to schedule a halving of the cycle
	for(int i = 0; i < AC; i++)
	{
		if(backlog.at(i) == 1)
		{
			if(counters.at(i) == 0)
			{	
				// Can we reduce the future cycle?
				if(changeStage.at(i) == 1)
				{
					//Resisting a colision because we changed to another schedule
					stationStickiness.at(i) = std::min(stationStickiness.at(i)+1, systemStickiness+1);
					//Reseting the control variables
					resetAttempt.at(i) = 0;
					consecutiveSx.at(i) = 0;
					changeStage.at(i) = 0;
					reset.at(i)++;
					//------------------------------

					//*/DEBUG
					cout << "**Node " << node << endl;
					cout << "\tMaking the change from stage " << stages.at(i);
					stages.at(i) = newStage;
					cout << " to " << stages.at(i) << " now" << endl;

				}

				//1. Checking the complete schedule
				resetThresholds.at(i) = ( (int)(pow(2, MAXSTAGES[i]) * CWmin[i] / 2) ) / (int)( pow(2, stages.at(i)) * CWmin[i] / 2 - 1 );

				//2. Checking half of the complete schedule (more aggresive)
				// resetThresholds.at(i) = ( (int)(pow(2, (MAXSTAGES[i]-1) ) * CWmin[i] / 2) / (int)(pow(2, stages.at(i)) * CWmin[i] / 2) ) -1;

				//3. Super aggressive
				// resetThresholds.at(i) = 1;

				if(resetThresholds.at(i) <= 0) resetThresholds.at(i) = 1;

				//*/DEBUG
					// cout << "**Node  " << node << endl;
					// cout << "\tNew threshold: " << resetThresholds.at(i) << endl;

				//If you have transmitted successfully for a whole complete cycle, you can starting the analysis
				if( ((int)consecutiveSx.at(i) >= (int)resetThresholds.at(i)) )
				{
					resetAttempt.at(i) = 1;
					// We will look at all the slots between transmissions
					resetCounters.at(i) = (pow(2,stages.at(i)) * CWmin[i]/2) -1;
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