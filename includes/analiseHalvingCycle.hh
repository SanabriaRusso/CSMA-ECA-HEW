#define AC 4

using namespace std;

void analiseHalvingCycle(std::array<double,AC> &consecutiveSx, std::array<double,AC> &halvingCounters,
	std::array<int,AC> &stages, std::array<double,AC> &counters, int acToTx, const int MAXSTAGES[AC],
	std::array<int,AC> backlog, std::array<int,AC> &halvingAttempt, int slot, std::array<int, AC> &shouldHalve,
	std::array<int,AC> &halvingThresholds, int node, std::array<int,AC> &changeStage, std::array<double,AC> &halved,
	std::array<int, AC> &stationStickiness, int systemStickiness){

	int CWmin [AC] = { 32, 32, 16, 8 }; //slots


	//Decrementing previously set counters
	for(int i = 0; i < AC; i++)
	{
		if(halvingAttempt.at(i) > 0)
		{
			if(halvingCounters.at(i) == 0)
			{
				shouldHalve.at(i) += slot;
				halvingCounters.at(i) = -1;	//waiting for another schedule of the halving
				if(shouldHalve.at(i) > 0)
				{
					halvingAttempt.at(i) = 0;	//Restart the looking around if one try fails
				}

				// cout << "**Node " << node << " saving slot state (" << slot << ")" << endl;
			}else if(halvingCounters.at(i) > 0)
			{
				halvingCounters.at(i)--;	
			}
			//*/DEBUG
			// if(halvingCounters.at(i) != -1) 
			// 	cout << "Node " << node << " slot #" << halvingCounters.at(i) << ", slot status: " <<  s << endl;
		}
	}

	//Can we halve the cycle?
	for(int i = 0; i < AC; i++)
	{
		if( (halvingCounters.at(i) == -1) && ( (int)halvingAttempt.at(i) == (int)halvingThresholds.at(i) ) ) //Checking if we can
		{
			if(shouldHalve.at(i) == 0) //our slot in the new cycle is free
			{
				//*/DEBUG
				// int previousStage = std::max(0, stages.at(i) - 1);
				// cout << "**Node " << node << endl;
				// cout << "Halving attempt #" << halvingAttempt.at(i) << endl;
				// cout << "\tChange stage from: " << stages.at(i) << " to: " << previousStage << endl;

				changeStage.at(i) = 1;
				halvingAttempt.at(i) = 0;
			}else
			{
				//*/DEBUG
				// cout << "**Node " << node << endl;
				// cout << "Don't change stage (" << shouldHalve.at(i) << ")" << endl;
				
				changeStage.at(i) = 0;
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
				if(changeStage.at(i) == 1) //Halve the cycle for the next transmission now
				{
					stages.at(i) = std::max(0, stages.at(i) - 1);
					stationStickiness.at(i) = std::min(stationStickiness.at(i)+1, systemStickiness+1);	//Resisting a colision because we changed to another schedule
					consecutiveSx.at(i) = 0;
					changeStage.at(i) = 0;
					halved.at(i)++;

					//*/DEBUG
					// cout << "**Node " << node << endl;
					// cout << "\tMaking the change to stage " << stages.at(i) << " now." << endl;

				}
				//1. Checking the complete schedule
				// halvingThresholds.at(i) = ( (int)(pow(2, MAXSTAGES[i]) * CWmin[i] / 2) / (int)(pow(2, stages.at(i)) * CWmin[i] / 2) ) -1;

				//2. Checking half of the complete schedule (more aggresive)
				halvingThresholds.at(i) = ( (int)(pow(2, (MAXSTAGES[i]-1) ) * CWmin[i] / 2) / (int)(pow(2, stages.at(i)) * CWmin[i] / 2) ) -1;

				//3. Super aggressive
				// halvingThresholds.at(i) = 1;

				if(halvingThresholds.at(i) == 0) halvingThresholds.at(i) = 1;

				//*/DEBUG
					// cout << "**Node  " << node << endl;
					// cout << "\tNew threshold: " << halvingThresholds.at(i) << endl;

				if( ((int)consecutiveSx.at(i) >= (int)halvingThresholds.at(i)) ) //you can start to schedule a halving
				{
					int previousStage = std::max(0, stages.at(i) - 1);
					halvingAttempt.at(i)++;
					halvingCounters.at(i) = (pow(2,previousStage) * CWmin[i]/2 + 1); 	//+1 because we want the information of the
																						//slot containing the actual transmission
					///*DEBUG
					// cout << "**Node " << node << " AC " << i << ": halveCounter: " << halvingCounters.at(i) << endl;
					// cout << "\tOwn transmition in the next slot, number: " << (pow(2, stages.at(i)) * CWmin[i] / 2) << "." << endl;
				}
			}
		}
	}
}