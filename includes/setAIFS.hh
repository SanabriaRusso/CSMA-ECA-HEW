#define AC 4

using namespace std;
void setAIFS(std::array<double,AC> &AIFS, int ECA, const int defaultAIFS[AC], const int ECA_AIFS[AC] )
{
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
			AIFS.at(i) = ECA_AIFS[i];
		}
	}
}