#include <math.h>
#include <algorithm>

#define AC 4
#define MAXSTAGE 5

Packet preparePacketForTransmission(int acToTx, double txTime, std::array<Packet,AC> &superPacket)
{
	superPacket.at(acToTx).tx_time = txTime;
	superPacket.at(acToTx).accessCategory = acToTx;

	return(superPacket.at(acToTx));
}