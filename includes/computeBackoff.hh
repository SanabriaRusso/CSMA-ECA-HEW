using namespace std;

void computeBackoff(int &backlog, double &qSize, int &AC, int &stickiness, int &backoffStage, 
	double &counter, int &system_stickiness, int &id, int &sx, int &EDCA){

	int CWmin = 0;

	//CWmin values extracted from Perahia & Stacey's: Next Generation Wireless LANs (p. 240)

	switch (AC){
		case 0:
			if(qSize > 0) CWmin = 32;
			break;
		case 1:
			if(qSize > 0) CWmin = 32;
			break;
		case 2:
			if(qSize > 0) CWmin = 16;
			break;
		case 3:
			if(qSize > 0) CWmin = 8;
			break;
		default:
			break;	
	}
	
	if(CWmin > 0)
	{
		//cout << "Node " << id << ". AC" << AC << " Old counter: " << counter << endl;

		if(sx == 1)
		{
			if(EDCA == 1)
			{
				counter = (int)(pow(2,backoffStage)*CWmin/2);
				//cout << "+++Node " << id << " AC: " << AC << " ECA: " << counter << endl;
			}else
			{
				counter = rand() % (int) ( (pow(2,backoffStage) * CWmin) );
				//cout << "---Node " << id << " AC: " << AC << " DCF: " << counter << endl;
			}
		}else
		{
			if(stickiness > 0)
			{
				counter = (int)(pow(2,backoffStage)*CWmin/2);
				//cout << "+++Node " << id << " AC: " << AC << " ECA (hyst): " << counter << endl;				
			}else
			{
				counter = rand() % (int) ( (pow(2,backoffStage) * CWmin) );
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