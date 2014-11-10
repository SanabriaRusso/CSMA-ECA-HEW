#include <math.h>
#include <algorithm>

#define AC 4
#define MAXSTAGE 5

using namespace std;

void preparePacketForTransmission(int acToTx, double txTime, Packet &packet)
{
	packet.tx_time = txTime;
	packet.accessCategory = acToTx;

}