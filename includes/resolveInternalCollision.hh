#define AC 4
#define MAXSTAGE 5

int resolveInternalCollision(std::array<double,AC> &counters, std::array<int,AC> &backlogg, std::array<int,AC> &stickiness, 
	std::array<int,AC> &stages, std::array<int,AC> &recomputeBackoff, std::array<double,AC> &totalInternalACCol,
	std::array<int,AC> &retAttemptAC){

	int iterator = counters.size() - 1;
	int acToTx;
	int winner = -1;

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

	acToTx = winner;

	if(acToTx > 0)
	{
		for(int i = 0; i < acToTx; i++)
		{
			if(backlogg.at(i) == 1)
			{
				if(counters.at(i) == 0)
				{
					stickiness.at(i) = std::max((int) stickiness.at(i) - 1, 0);
					stages.at(i) = std::min((int)stages.at(i) + 1, MAXSTAGE);
				
					recomputeBackoff.at(i) = 1;
					totalInternalACCol.at(i)++;

					retAttemptAC.at(i)++;
				}else
				{
					recomputeBackoff.at(i) = 0;
				}
			}
		}
	}



	return(acToTx);
	
}