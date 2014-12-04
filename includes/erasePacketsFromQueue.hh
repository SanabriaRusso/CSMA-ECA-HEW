#include "../Aux.h"

using namespace std;

void erasePacketsFromQueue(FIFO <Packet> &MACQueueBK, FIFO <Packet> &MACQueueBE, FIFO <Packet> &MACQueueVI, 
    FIFO <Packet> &MACQueueVO, Packet &packet, int id, int backlogged)
{
    int packetDisposal = 0;
    FIFO <Packet> *Q;

    switch (packet.accessCategory)
    {
        case 0:
            Q = &MACQueueBE;
            break;

        case 1:
            Q = &MACQueueBK;
            break;

        case 2:
            Q = &MACQueueVI;
            break;

        case 3:
            Q = &MACQueueVO;
            break;
    }

    packetDisposal = std::min(packet.aggregation, Q->QueueSize());
    //cout << packetDisposal << endl;
    //cout << "Old queue: " << Q.QueueSize() << endl;
    for(int i = 0; i < packetDisposal; i++) Q->DelFirstPacket();
    //cout << "New queue: " << Q.QueueSize() << endl;

    if (Q->QueueSize() > 0)
    {   
        backlogged = 1;
    }else
    {
        backlogged = 0;
    }

}