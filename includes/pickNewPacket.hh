#include "../Aux.h"
#define AC 4

using namespace std;

void pickNewPacket(int &accessCategory, double pickupTime, std::array<Packet,AC> &superPacket, 
	std::array<FIFO <Packet>, AC> &Queues, int id, std::array<int,AC> stages, int fairShare)

{
	Packet packet = Queues.at(accessCategory).GetFirstPacket();
	packet.contention_time = pickupTime;
	superPacket.at(accessCategory).contention_time = packet.contention_time;
	superPacket.at(accessCategory).startContentionStage = stages.at(accessCategory);
	
	if(fairShare == 1)
	{		
		superPacket.at(accessCategory).aggregation = std::min( (int)pow(2,stages.at(accessCategory)), 
			Queues.at(accessCategory).QueueSize() );
	}else
	{
		superPacket.at(accessCategory).aggregation = 1;
	}
}
