#define AC 4

using namespace std;

void computeBackoff(int &backlog, FIFO <Packet> &Queue, int &ac, int &stickiness, int &backoffStage, 
	double &counter, int &system_stickiness, int &id, int &sx, int &ECA, double &AIFS, const int defaultAIFS[AC]){

	//CWmin values extracted from Perahia & Stacey's: Next Generation Wireless LANs (p. 240)
	int CWmin [4] = { 32, 32, 16, 8 };

	// int CWmin [4] = { 64, 64, 32, 16 };
	// int CWmin [4] = { 128, 128, 64, 32 };
	// int CWmin [4] = { 1024, 1024, 1024, 1024 };

	if(backlog == 1)
	{
		//cout << "Node " << id << ". AC" << AC << " Old counter: " << counter << endl;
		if(sx == 1)
		{
			if(ECA == 1)
			{
				counter = (int)((pow(2,backoffStage)*CWmin[ac]/2)-1);

				// cout << "+++Node " << id << " AC: " << ac << " ECA: " << counter << endl;
			}else
			{
				counter = rand() % (int)  (pow(2,backoffStage) * CWmin[ac] );
				// cout << "---Node " << id << " AC: " << ac << " DCF: " << counter << endl;
			}
		}else
		{
			if(stickiness > 0)
			{
				counter = (int)((pow(2,backoffStage)*CWmin[ac]/2)-1);
				// cout << "+++Node " << id << " AC: " << AC << " ECA (hyst): " << counter << endl;				
			}else
			{
				counter = rand() % (int) (pow(2,backoffStage) * CWmin[ac] );
				// cout << "---Node " << id << " AC: " << AC << " DCF (col): " << counter << endl;
			}
			
		}
	}else
	{
		backoffStage = 0;
		counter = 0;
		stickiness = system_stickiness;
		// cout << "\tAC " << ac << " has an empty queue" << endl;
	}
	AIFS = defaultAIFS[ac];
}