#define AC 4

using namespace std;

void decrement(int category, double &counter, double &AIFS)
{
	if (AIFS > 0)
	{
		AIFS--;
	}else
	{
		counter--;
	}

	cout << "AC " << category << ": AIFS " << AIFS << ", Counter " << counter << endl;

}