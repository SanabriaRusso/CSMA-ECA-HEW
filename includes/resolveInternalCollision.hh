#define AC 4

int resolveInternalCollision(std::array<double,AC> &counters, std::array<int,AC> &backlogg, std::array<int,AC> &stickiness, 
	std::array<int,AC> &stages, std::array<int,AC> &recomputeBackoff, std::array<double,AC> &totalInternalACCol,
	std::array<int,AC> &retAttemptAC, int scheme, int id, const int MAXSTAGE_ECA[AC], const int MAXSTAGE_EDCA[AC], 
	std::array<double,AC> &consecutiveSx, double timer){

	int iterator = counters.size() - 1;
	int acToTx;
	int winner = -1;

	recomputeBackoff.fill(0);

	// cout << "Internal collision check" << endl;
	// for (int i = 0; i < AC; i++)
	// {
	// 	cout << "\tAC " << i << ": " << counters.at(i) <<  endl;
	// }

	for(int i = iterator; i >= 0; i--)
	{
		if(backlogg.at(i) == 1)
		{ 
			if(counters.at(i) == 0) 
			{
				winner = i; 
				break;
			}
		}
	}

	// cout << "\tWinner AC " << winner << endl;

	acToTx = winner;


	if(winner >= 0)
	{
		for(int i = 0; i < winner; i++)
		{
			if(backlogg.at(i) == 1)
			{
				if(counters.at(i) == 0)
				{
					int recompute = i;
					recomputeBackoff.at(recompute) = 1;
					totalInternalACCol.at(recompute)++;
					
					consecutiveSx.at(recompute) = 0;

					// cout << "\n(" << timer << ") STA-" << id << ": ECA Internal collision" << endl;
					// cout << "---AC " << i << " counter: " << counters.at(i) <<  endl;
					
					if(scheme == 0) //EDCA
					{
						stickiness.at(recompute) = std::max((int) stickiness.at(recompute) - 1, 0);
						stages.at(recompute) = std::min((int)stages.at(recompute) + 1, MAXSTAGE_EDCA[recompute]);

						// cout << "\nInternal collision" << endl;
						// cout << "\t---Changing " << recompute << endl;
						retAttemptAC.at(recompute)++;
					}
				}
			}
		}
	}

	return(acToTx);
	
}