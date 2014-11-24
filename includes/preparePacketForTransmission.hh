#include <math.h>
#include <algorithm>

#define AC 4
#define MAXSTAGE 5

Packet preparePacketForTransmission(int acToTx, double txTime, std::array<Packet,AC> &superPacket, int id)
{
	superPacket.at(acToTx).source = id;
	superPacket.at(acToTx).tx_time = txTime;
	superPacket.at(acToTx).accessCategory = acToTx;
	superPacket.at(acToTx).aggregation = 1;

	return((Packet)(superPacket.at(acToTx)));
}