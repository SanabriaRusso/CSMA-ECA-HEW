#include "../Aux.h"
#define AC 4

void pickNewPacket(int &accessCategory, double pickupTime, std::array<Packet,AC> &superPacket, 
	std::array<FIFO <Packet>, AC> &Queues, int id)

{
	Packet packet = Queues.at(accessCategory).GetFirstPacket();
	packet.contention_time = pickupTime;
	superPacket.at(accessCategory).contention_time = packet.contention_time;
}
