#define AC 4

using namespace std;


void setAIFS(std::array<double,AC> &AIFS, int ECA, const int defaultAIFS[AC] )
{
	int emptyAIFS [AC] = {0,0,0,0};

	if(ECA == 0)	//EDCA
	{	
		for(int i = 0; i < AC; i++)
		{
			AIFS.at(i) = defaultAIFS[i];
		}
	}else
	{
		for(int i = 0; i < AC; i++)
		{
			AIFS.at(i) = emptyAIFS[i];
		}
	}
	

}