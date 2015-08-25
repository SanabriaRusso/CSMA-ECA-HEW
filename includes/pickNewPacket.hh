#include "../Aux.h"
#define AC 4

using namespace std;

void pickNewPacket(int &accessCategory, double pickupTime, std::array<Packet,AC> &superPacket, 
	std::array<FIFO <Packet>, AC> &Queues, int id, std::array<int,AC> stages, int fairShare, int maxAggregation, 
	const int MAXSTAGE_EDCA[AC], const int MAXSTAGE_ECA[AC], int ECA)

{
	Packet packet = Queues.at(accessCategory).GetFirstPacket();
	packet.contention_time = pickupTime;
	superPacket.at(accessCategory).contention_time = packet.contention_time;
	superPacket.at(accessCategory).startContentionStage = stages.at(accessCategory);
	superPacket.at(accessCategory).fairShare = fairShare; //to decide tx duration in channel.hh
	
	int packets = 1;
	if(fairShare == 1)
	{
		packets = std::min( (int)pow(2,stages.at(accessCategory)), Queues.at(accessCategory).QueueSize() );
	}

	if(maxAggregation == 1)
	{
		int exponent = MAXSTAGE_EDCA[accessCategory];
		if(ECA == 1) exponent = MAXSTAGE_ECA[accessCategory];
		packets = std::min( (int)pow(2,exponent), Queues.at(accessCategory).QueueSize() );
	}

	superPacket.at(accessCategory).aggregation = packets;
}
