using namespace std;

//ECA = 0 means EDCA

void selectMACProtocol(const int node_id, int &ECA, int stickiness, int cut){


	// if(ECA > 0)
	// {
	// 	ECA = 1;
	// }else
	// {
	// 	ECA = 0;
	// }
	if(cut == 0)
	{
		ECA = 1;
		return;
	}

	if(node_id < cut)
	{
		//cout << node_id << ": I am using EDCA";
		ECA = 1;
	}else
	{
		//cout << node_id << ": I am using CSMA/ECA" << endl;
		ECA = 0;
	}
}