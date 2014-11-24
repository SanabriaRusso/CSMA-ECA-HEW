using namespace std;

void erasePacketsFromQueue(FIFO <Packet> &MACQueueBK, FIFO <Packet> &MACQueueBE, FIFO <Packet> &MACQueueVI, FIFO <Packet> &MACQueueVO, Packet &packet, int id)

{
    int packetDisposal = 0;

    switch (packet.accessCategory)
    {
        case 0:
            packetDisposal = std::min(packet.aggregation, MACQueueBE.QueueSize());
            //cout << packetDisposal << endl;
            //cout << "Old queue: " << MACQueueBE.QueueSize() << endl;
            for(int i = 0; i < packetDisposal; i++) MACQueueBE.DelFirstPacket();
            //cout << "New queue: " << MACQueueBE.QueueSize() << endl;
            break;
        case 1:
            packetDisposal = std::min(packet.aggregation, MACQueueBK.QueueSize());
            //cout << packetDisposal << endl;
            //cout << "Old queue: " << MACQueueBE.QueueSize() << endl;
            for(int i = 0; i < packetDisposal; i++) MACQueueBK.DelFirstPacket();
            //cout << "New queue: " << MACQueueBK.QueueSize() << endl;
            break;
        case 2:
            packetDisposal = std::min(packet.aggregation, MACQueueVI.QueueSize());
            //cout << packetDisposal << endl;
            //cout << "Old queue: " << MACQueueVI.QueueSize() << endl;
            for(int i = 0; i < packetDisposal; i++) MACQueueVI.DelFirstPacket();
            //cout << "New queue: " << MACQueueVI.QueueSize() << endl;

            break;
        case 3:
            packetDisposal = std::min(packet.aggregation, MACQueueVO.QueueSize());
            //cout << packetDisposal << endl;
            //cout << "Old queue: " << MACQueueVO.QueueSize() << endl;
            for(int i = 0; i < packetDisposal; i++) MACQueueVO.DelFirstPacket();
            //cout << "New queue: " << MACQueueVO.QueueSize() << endl;
            break;
    }
}