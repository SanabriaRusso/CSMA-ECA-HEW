using namespace std;

//ECA = 0 means EDCA

void selectMACProtocol(const int node_id, int &ECA, int stickiness){


	if(ECA > 0)
	{
		ECA = 1;
	}else
	{
		ECA = 0;
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