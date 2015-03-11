#define AC 4

using namespace std;

void analiseHalvingCycle(std::array<double,AC> &consecutiveSx, std::array<double,AC> &halvingCounters,
	std::array<int,AC> &stages, std::array<double,AC> &counters, int acToTx, const int MAXSTAGES[AC],
	std::array<int,AC> backlog, std::array<int,AC> &halvingAttempt, int s, std::array<int, AC> &shouldHalve,
	std::array<int,AC> &halvingThresholds, int node, std::array<int,AC> &changeStage){

	int CWmin [AC] = { 32, 32, 16, 8 }; //slots
	int slot;
	if(s > 0)
	{
		slot = 0;
	}else
	{
		slot = 1; 
	}


	//Decrementing previously set counters
	for(int i = 0; i < AC; i++)
	{
		if(halvingAttempt.at(i) > 0)
		{
			if(halvingCounters.at(i) == 0)
			{
				shouldHalve.at(i) *= slot;
				halvingCounters.at(i) = -1;	//waiting for another schedule of the halving
				// cout << "**Node " << node << " saving slot state (" << s << ")" << endl;
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
		if( (int)halvingAttempt.at(i) == ((int)halvingThresholds.at(i) - 1) )
		{
			if(shouldHalve.at(i) > 0)
			{
				//*/DEBUG
				// int previousStage = std::max(0, stages.at(i) - 1);
				// cout << "**Node " << node << endl;
				// cout << "Halving attempt #" << halvingAttempt.at(i) << endl;
				// cout << "Change stage (" << shouldHalve.at(i) << ")" << endl;
				// cout << "\tChange stage from: " << stages.at(i) << " to: " << previousStage << endl;

				halvingAttempt.at(i) = 0;
				changeStage.at(i) = 1;
			}else
			{
				//*/DEBUG
				// cout << "**Node " << node << endl;
				// cout << "Don't change stage (" << shouldHalve.at(i) << ")" << endl;
				
				halvingAttempt.at(i) = 0;
				changeStage.at(i) = 0;
				consecutiveSx.at(i) = 0;
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
					changeStage.at(i) = 0;

					//*/DEBUG
					// cout << "**Node " << node << endl;
					// cout << "Making the change to stage " << stages.at(i) << " now." << endl;

				}

				halvingThresholds.at(i) = (pow(2, MAXSTAGES[i]) * CWmin[i] / 2) / (pow(2, stages.at(i)) * CWmin[i] / 2);
				if( ((int)consecutiveSx.at(i) >= (int)halvingThresholds.at(i)) ) //you can start to schedule a halving
				{
					int previousStage = std::max(0, stages.at(i) - 1);
					halvingAttempt.at(i)++;
					halvingCounters.at(i) = (pow(2,previousStage) * CWmin[i]/2 + 1); 	//+1 because we want the information of the
																						//slot containing the actual transmission
					///*DEBUG
					// cout << "**Node " << node << " AC " << i << ": halveCounter: " << halvingCounters.at(i) << endl;
					// cout << "\tExpect transmition in the: " << (pow(2, stages.at(i)) * CWmin[i] / 2) << "th slot." << endl;
				}
			}
		}
	}
}