using namespace std;

//ECA = 0 means EDCA

void selectMACProtocol(const int node_id, int &ECA, int stickiness, int cut, int fairShare){
	switch(cut)
	{
		case 0:
			ECA = 1;
			if(stickiness == 0) ECA = 3;
			break;
		case 1:
			ECA = 0;
			break;
		default:
			if(node_id < cut)
			{	
				ECA = 1;
			}else
			{
				ECA = 0;
			}
	}
	if (ECA == 0 && fairShare == 1)
	{
		ECA = 0;
	}
}