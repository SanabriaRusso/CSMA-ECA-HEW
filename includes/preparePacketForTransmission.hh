#include <math.h>
#include <algorithm>

#define AC 4
#define MAXSTAGE 5

Packet preparePacketForTransmission(int acToTx, double txTime, std::array<Packet,AC> &superPacket, 
	int id, std::array<int,AC> stages)
{
	superPacket.at(acToTx).source = id;
	superPacket.at(acToTx).tx_time = txTime;
	superPacket.at(acToTx).accessCategory = acToTx;
	superPacket.at(acToTx).aggregation = pow(2,stages.at(acToTx));

	return((Packet)(superPacket.at(acToTx)));
}