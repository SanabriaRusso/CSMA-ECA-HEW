#define AC 4

using namespace std;

void computeBackoff(int &backlog, FIFO <Packet> &Queue, int &ac, int &stickiness, int &backoffStage, 
	double &counter, int &system_stickiness, int &id, int &sx, int &EDCA){

	int CWmin [4] = { 32, 32, 16, 8 };

	//CWmin values extracted from Perahia & Stacey's: Next Generation Wireless LANs (p. 240)

	if(Queue.QueueSize() > 0)
	{
		//cout << "Node " << id << ". AC" << AC << " Old counter: " << counter << endl;

		if(sx == 1)
		{
			if(EDCA == 1)
			{
				counter = (int)(pow(2,backoffStage)*CWmin[ac]/2);
				//cout << "+++Node " << id << " AC: " << AC << " ECA: " << counter << endl;
			}else
			{
				counter = rand() % (int)  (pow(2,backoffStage) * CWmin[ac] );
				//cout << "---Node " << id << " AC: " << AC << " DCF: " << counter << endl;
			}
		}else
		{
			if(stickiness > 0)
			{
				counter = (int)(pow(2,backoffStage)*CWmin[ac]/2);
				//cout << "+++Node " << id << " AC: " << AC << " ECA (hyst): " << counter << endl;				
			}else
			{
				counter = rand() % (int) (pow(2,backoffStage) * CWmin[ac] );
				//cout << "---Node " << id << " AC: " << AC << " DCF (col): " << counter << endl;
			}
			
		}
		backlog = 1;
		
	}else
	{
		backlog = 0;
		backoffStage = 0;
		counter = 0;
		stickiness = system_stickiness;
		//cout << "\tAC " << AC << " has an empty queue" << endl;
	}
}