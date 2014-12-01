#include "../Aux.h"
#define AC 4

void pickNewPacket(int &accessCategory, double pickupTime, std::array<Packet,AC> &superPacket, FIFO <Packet> &MACQueueBK, 
	FIFO <Packet> &MACQueueBE, FIFO <Packet> &MACQueueVI, FIFO <Packet> &MACQueueVO, int id)
{
	Packet packet;
	switch(accessCategory)
	{
		case 0:
			packet = MACQueueBE.GetFirstPacket();
			break;
		case 1:
			packet = MACQueueBK.GetFirstPacket();
			break;
		case 2:
			packet = MACQueueVI.GetFirstPacket();
			break;
		case 3:
			packet = MACQueueVO.GetFirstPacket();
			break;

	}
	packet.contention_time = pickupTime;
	superPacket.at(accessCategory).contention_time = packet.contention_time;
}