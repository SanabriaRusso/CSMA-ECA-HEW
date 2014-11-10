#include <math.h>
#include <algorithm>

#define AC 4
#define MAXSTAGE 5

void preparePacketForTransmission(int acToTx, double txTime, Packet &packet)
{
	packet.tx_time = txTime;
	packet.accessCategory = acToTx;

}