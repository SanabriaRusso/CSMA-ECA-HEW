using namespace std;

//EDCA = 0 means CSMA/ECA

void selectMACProtocol(const int node_id, int &EDCA, int stickiness){


	if(EDCA > 0)
	{
		EDCA = 1;
	}else
	{
		EDCA = 0;
	}


	/*if(node_id < cut)
	{
		//cout << node_id << ": I am using EDCA";
		EDCA = 1;
		hysteresis = 0;
		//Set to 1 when trying maximum aggregation in mixed scenario. 0 Otherwise
		if(maxAggregation > 0)
		{
			fairShare = 1;
			//cout << " with maximum aggregation" << endl;
		}else
		{
			fairShare = 0;
			//cout << " without aggregation" << endl;
		}
	}else
	{
		//cout << node_id << ": I am using CSMA/ECA" << endl;
		EDCA = 0;
	}*/
}