#include "../Aux.h"

using namespace std;

void erasePacketsFromQueue(std::array<FIFO <Packet>, AC> &Queues, Packet &packet, int id, 
    int backlogged, int fairShare, int sx, double &dropped)
{
    int packetDisposal = 0;

    if(packet.accessCategory >= 0)
    {
        if(sx == 1 || fairShare == 0)
        {
            packetDisposal = std::min( packet.aggregation, 
                Queues.at(packet.accessCategory).QueueSize() );

            //cout << "2) STA-" << id << " Success. Erasing: " << packetDisposal << endl;
        }else
        {
            packetDisposal = std::min((int)pow(2, packet.startContentionStage), 
                Queues.at(packet.accessCategory).QueueSize());

            //cout << "STA-" << id << " Dropping. Erasing: " << packetDisposal << endl;

            dropped+= packetDisposal;
        }
        

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

}
