#include <math.h>
#include <algorithm>

#define AC 4

Packet preparePacketForTransmission(int acToTx, double txTime, std::array<Packet,AC> &superPacket, 
	int id, std::array<int,AC> &stages, std::array<FIFO <Packet>, AC> &Queues, int fairShare, int ECA)
{
	superPacket.at(acToTx).source = id;
	superPacket.at(acToTx).tx_time = txTime;
	superPacket.at(acToTx).accessCategory = acToTx;
	superPacket.at(acToTx).QoS = 0;

	if(fairShare == 1)
	{		
		superPacket.at(acToTx).aggregation = std::min( (int)pow(2,stages.at(acToTx)), 
			Queues.at(acToTx).QueueSize() );
		superPacket.at(acToTx).QoS = 1;
	}else
	{
		superPacket.at(acToTx).aggregation = 1;
	}

	int limit = superPacket.at(acToTx).aggregation;
	superPacket.at(acToTx).allSeq.assign (limit,0);
	long int load = 0;
	for(int i = 0; i < limit; i++)
	{	
		load += Queues.at(superPacket.at(acToTx).accessCategory).GetPacket(i).L;
		superPacket.at(acToTx).allSeq.at (i) = Queues.at(superPacket.at(acToTx).accessCategory).GetPacket(i).seq;
	}
	superPacket.at(acToTx).L = load;

	// cout << "Sending: " << superPacket.at(acToTx).aggregation << endl;

	return((Packet)(superPacket.at(acToTx)));
}