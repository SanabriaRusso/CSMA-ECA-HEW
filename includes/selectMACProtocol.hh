using namespace std;

//ECA = 0 means EDCA
/* This needs to be rewritten. It is a quick patch */

void selectMACProtocol(const int node_id, int &ECA, int stickiness, int fairShare, float edcaShare, int nodes){
	
	if (edcaShare == 1 || edcaShare == 0)
	{
		if (edcaShare == 1)
			ECA = 0;
		if (edcaShare == 0)
			ECA = 1;
	}else
	{
		if (node_id < (int)(edcaShare * nodes) )
		{
			ECA = 0;
		}else
		{
			ECA = 1;
		}
	}

	if (ECA == 0 && fairShare == 1)
	{
		ECA = 0;
	}
}