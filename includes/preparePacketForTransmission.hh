#include <math.h>
#include <algorithm>

#define AC 4
#define MAXSTAGE 5

Packet preparePacketForTransmission(int acToTx, double txTime, std::array<Packet,AC> &superPacket, 
	int id, std::array<int,AC> stages, std::array<FIFO <Packet>, AC> &Queues, int fairShare)
{
	superPacket.at(acToTx).source = id;
	superPacket.at(acToTx).tx_time = txTime;
	superPacket.at(acToTx).accessCategory = acToTx;

	if(fairShare == 1)
	{		
		superPacket.at(acToTx).aggregation = std::min( (int)pow(2,stages.at(acToTx)), 
			Queues.at(acToTx).QueueSize() );
	}else
	{
		superPacket.at(acToTx).aggregation = 1;
	}

	return((Packet)(superPacket.at(acToTx)));
}