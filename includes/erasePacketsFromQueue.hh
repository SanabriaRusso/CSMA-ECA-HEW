#include "../Aux.h"

using namespace std;

void erasePacketsFromQueue(std::array<FIFO <Packet>, AC> &Queues, Packet &packet, int id, 
    int &backlogged, int fairShare, int sx, double &dropped, std::array<double,AC> &qEmpty, int &affected,
    double &qDelay, double now, bool &alwaysSat, double &bitsSentByAc, double &bitsFromSuperPacket, 
    std::vector<int> &errorInFrame, bool &TXOP, int ECA)
{
    int packetDisposal = 0;
    int aggregation = (int)packet.aggregation;
    int cat = (int)packet.accessCategory;
    int frames = 1;

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
                if (TXOP)
                {
                    packetDisposal = std::min (aggregation, (int)Queues.at(cat).QueueSize() );

                }else
                {
                    if (cat > 1)
                        frames = pow(2,packet.startContentionStage);
                    packetDisposal = std::min( (int)frames, (int)Queues.at(cat).QueueSize() );
                }
            }else
            {
                assert (aggregation == 1);
                packetDisposal = std::min( aggregation, (int)Queues.at(cat).QueueSize() );
            }
            dropped+= packetDisposal;
        }
        
        double bits = 0.0; //local debug variable
        FIFO <Packet> Q;
        Packet pkt;
        for (int i = 0; i < packetDisposal; i++)
        {
            Packet pkt = Queues.at(cat).GetFirstPacket();
            if (sx == 1)
            {  
                if (errorInFrame.at(i) == 1) 
                {
                    Q.PutPacket(pkt);
                }else{
                    bits += pkt.L * 8;
                    bitsSentByAc += pkt.L * 8;
                    qDelay += now - pkt.queuing_time;
                }
            }
            Queues.at(cat).DelFirstPacket ();
            if (alwaysSat) // putting the packet back at the back of the queue
            {
                // if (Queues.at(cat).QueueSize() > packetDisposal)
                // {
                //     assert (pkt.queuing_time != now);
                //     assert (Queues.at(cat).GetFirstPacket().seq != pkt.seq);
                // }
                pkt.queuing_time = now;
                Queues.at(cat).PutPacket (pkt);
                // cout << Queues.at(cat).GetFirstPacket().seq << " " << pkt.seq << endl;
            }
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
