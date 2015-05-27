#include "../Aux.h"

using namespace std;

void erasePacketsFromQueue(std::array<FIFO <Packet>, AC> &Queues, Packet &packet, int id, 
    int &backlogged, int fairShare, int sx, double &dropped, std::array<double,AC> &qEmpty, int &affected)
{
    int packetDisposal = 0;
    int aggregation = (int)packet.aggregation;
    int cat = (int)packet.accessCategory;

    if(cat >= 0)
    {
        if(sx == 1)
        {
            packetDisposal = std::min( (aggregation - affected), (int)Queues.at(cat).QueueSize() );

            // if(packetDisposal == 0) 
            // {
            //     cout << "*****ALARM: " << id << endl;
            // }
            // cout << "STA-" << id << " Success. Erasing: " << packetDisposal << endl;
            // cout << "STA-" << id << " aggregation: " << aggregation << " affected: " << affected << ", Q: " <<  (int)Queues.at(cat).QueueSize() << endl;
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

            // if(packetDisposal == 0)
            // {
            //     cout << "*****ALARM: " << id << endl;
            // }
            // cout << "STA-" << id << " Dropping. Erasing: " << packetDisposal << endl;
            

            dropped+= packetDisposal;
        }
        
        // cout << "\tOld queue: " << Queues.at(packet.accessCategory).QueueSize() << endl;
        for(int i = 0; i < packetDisposal; i++) Queues.at(cat).DelFirstPacket();
        // cout << "\tNew queue: " << Queues.at(packet.accessCategory).QueueSize() << endl;


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
