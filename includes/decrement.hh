#define AC 4

using namespace std;

void decrement(int category, double &counter, double &AIFS, int station, double timer)
{
	if(AIFS == 0)
	{
		counter --;
	}else
	{
		AIFS --;
	}

	// cout << "(" << timer << ") STA-" << station << " AC " << category << ": AIFS " << AIFS << ", Counter " << counter << endl;
}