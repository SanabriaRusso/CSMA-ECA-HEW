#include "../Aux.h"

using namespace std;

void erasePacketsFromQueue(std::array<FIFO <Packet>, AC> &Queues, Packet &packet, int id, int backlogged)
{
    int packetDisposal = 0;

    packetDisposal = std::min(packet.aggregation, Queues.at(packet.accessCategory).QueueSize());
    //cout << packetDisposal << endl;
    //cout << "Old queue: " << Queues.at(packet.accessCategory).QueueSize() << endl;
    for(int i = 0; i < packetDisposal; i++) Queues.at(packet.accessCategory).DelFirstPacket();
    //cout << "New queue: " << Queues.at(packet.accessCategory).QueueSize() << endl;

    if (Queues.at(packet.accessCategory).QueueSize() > 0)
    {   
        backlogged = 1;
    }else
    {
        backlogged = 0;
    }

}
