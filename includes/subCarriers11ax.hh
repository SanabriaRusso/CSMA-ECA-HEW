using namespace std;

int subCarriers11ax (int bandwidth){
	int Ysb;
	switch (bandwidth) 
	{
	    case 20: 
	        Ysb = 52;
	        break;
	    case 40:
	        Ysb = 108;    
	        break;
	    case 80:
	        Ysb = 234;
	        break;
	    case 160:
	        Ysb = 468;
    		break;
		default:
			Ysb = 52;
			break;
	}
	return Ysb;
}