using namespace std;

int subCarriers11ax (int bandwidth){
	int Ysb;
	switch (bandwidth) 
	{
	    case 20:
	        Ysb = 234;
	    case 40:
	        Ysb = 468;    
	    case 80:
	        Ysb = 980;
	    case 160:
	        Ysb = 1960;
		default:
			Ysb = 234;
			break;
	}
	return Ysb;
}