#include "../Aux.h"

using namespace std;

void erasePacketsFromQueue(std::array<FIFO <Packet>, AC> &Queues, Packet &packet, int id, 
    int &backlogged, int fairShare, int sx, double &dropped, std::array<double,AC> &qEmpty, int &affected,
    double &qDelay, double now, bool &alwaysSat, double &bitsSentByAc, double &bitsFromSuperPacket, 
    std::vector<int> errorInFrame)
{
    int packetDisposal = 0;
    int aggregation = (int)packet.aggregation;
    int cat = (int)packet.accessCategory;

    if(cat >= 0)
    {
        if(sx == 1)
        {
            packetDisposal = std::min (aggregation, (int)Queues.at(cat).QueueSize());
            assert(packetDisposal == packet.allSeq.size ());
        }else
        {
            if(fairShare == 1)
            {
                packetDisposal = std::min( (int)pow(2, packet.startContentionStage), 
                    (int)Queues.at(cat).QueueSize() );
            }else
            {
                packetDisposal = std::min( aggregation, (int)Queues.at(cat).QueueSize() );
            }
            dropped+= packetDisposal;
        }
        
        double bits = 0.0; //local debug variable
        FIFO <Packet> Q;
        for (int i = 0; i < packetDisposal; i++)
        {
            Packet pkt;
            if (sx == 1)
            {
                pkt = Queues.at(cat).GetFirstPacket();    
                if (errorInFrame.at(i) == 1) 
                {
                    Q.PutPacket(pkt);
                }else{
                    bits += pkt.L;
                    bitsSentByAc += pkt.L * 8;
                    qDelay += now - pkt.queuing_time;
                }
            }
            if(!alwaysSat) Queues.at(cat).DelFirstPacket ();
        }
        if (Q.QueueSize () > 0)
            Queues.at(cat).PushFront (Q);
        assert(Q.QueueSize () == 0);

        if (Queues.at(cat).QueueSize() > 0)
        {   
            backlogged = 1;
        }else
        {
            backlogged = 0;
            qEmpty.at(cat)++;
        }
    }

}
