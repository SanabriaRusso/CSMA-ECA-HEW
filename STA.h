#include <time.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <math.h>
#include <algorithm>
#include <array>
#include "Aux.h"
#include "FIFO.h"
#include "includes/computeBackoff.hh"
#include "includes/selectMACProtocol.hh"
#include "includes/resolveInternalCollision.hh"
#include "includes/preparePacketForTransmission.hh"
#include "includes/erasePacketsFromQueue.hh"
#include "includes/pickNewPacket.hh"

//#define CWMIN 16 //to comply with 802.11n it should 16. Was 32 for 802.11b.
#define MAXSTAGE 5

//Suggested value is MAXSTAGE+1
#define MAX_RET 6

//Number of access categories
#define AC 4


using namespace std;

component STA : public TypeII
{
    public:
        void Setup();
        void Start();
        void Stop();

    public:
    	//Node-specific characteristics
        int node_id;
        int K; //max queue size
        int system_stickiness; //global stickiness
        std::array<int, AC> stationStickiness; //the stickiness of each AC
        int fairShare;
        
        //Protocol picking
        int cut;
        int EDCA;
        
        //aggregation settings
        int maxAggregation;
        int L; //length of packet
        
        //Source and Queue management
        double incommingPackets;
        std::array<double,AC> blockedPackets;
        std::array<double,AC> queuesSizes;
        std::array<double,AC> packetsInQueue;


        //Transmissions statistics
        std::array<double,AC> transmissions;
        std::array<double,AC> successfulTx; //successful transmissions per AC
        double overallSxTx;
        std::array<double,AC> overallACThroughput;
        double overallThroughput;

        //Collision stsatistics
        double totalCollisions;
        std::array<double,AC> totalACCollisions;

        double totalInternalCol;
        std::array<double,AC> totalInternalACCol;

        double totalRetransmissions;
        std::array<double,AC> totalACRet;
        std::array<int,AC> retAttemptAC;
        double totalDropped;
        std::array<double,AC> droppedAC;

        //Time statistics
        std::array<double,AC> accumTimeBetweenSxTx;

    private:
    	/*the positions in the backoff counters and stages vectors follow the 
    	ACs priorities, meaning: 0 = BE, 1 = BK, 2 = VI, 3 = VO*/
        std::array<double, AC> backoffCounters;
        std::array<int, AC> backoffStages;
        
        //Transmission private statistics
        int transmitted;    //0 -> no, 1 -> yes.
        int sx;             //0 -> no, 1 -> yes.

        /*For better understanding, the ACs are defined as constants for navigating
        the arrays*/
        int BE; //Best-effort
        int BK; //Background
        int VI; //Video
        int VO; //Voice
        int ACToTx; //to dertermine which AC is to transmit in case of an internal collision
        
        std::array<int,AC> backlogged; //whether or not the station has something to transmit on an AC (0 no, 1 yes)

		Packet packet; //individual packets
        std::array<Packet,AC> superPacket; //an abstract packet composed of #AC packets. This structure helps at keeping track of AC-related timers
        
        FIFO <Packet> MACQueueBK;
        FIFO <Packet> MACQueueBE;
        FIFO <Packet> MACQueueVI;
        FIFO <Packet> MACQueueVO;

    public:
        //Connections
        inport void inline in_slot(SLOT_notification &slot);
        inport void inline in_packet(Packet &packet);
        outport void out_packet(Packet &packet);
};

void STA :: Setup()
{

};

void STA :: Start()
{
	selectMACProtocol(node_id, EDCA, system_stickiness);

    //cout << EDCA << endl;
	
	/*Initializing variables and arrays to avoid warning regarding
	in-class initialization of non-static data members*/
	incommingPackets = 0;

    for(int i = 0; i < superPacket.size(); i++)
    {
        superPacket.at(i).contention_time = SimTime();
        //Setting the default aggregation parameter for the first packet transmission
        superPacket.at(i).aggregation = 1;
        superPacket.at(i).L = L;
    }


	BE = 0;
    BK = 1;
    VI = 2;
    VO = 3;
    ACToTx = -1;
    transmitted = 0;
    sx = 0;

    for(int i = 0; i < AC; i++)
    {
        stationStickiness.at(i) = system_stickiness; //Could individual AC stickiness parameter be interesting?
        droppedAC.at(i) = 0;
        backlogged.at(i) = 0;
        backoffStages.at(i) = 0;
        computeBackoff(backlogged.at(i), queuesSizes.at(i), i, stationStickiness.at(i), backoffStages.at(i), 
            backoffCounters.at(i), system_stickiness, node_id, sx, EDCA);
    }
	
};

void STA :: Stop()
{
    cout << "\n***Node-" << node_id << "**" <<endl;
    for(int it = 0; it < AC; it++)
    {
        cout << "AC " << it << endl;
        cout << "\t* Final Stickiness: " << stationStickiness.at(it) << " (system's: " << system_stickiness << ")." << endl;
        cout << "\t+ Total successfulTx for AC " << it << ": " << successfulTx.at(it) << endl;
        
        if(successfulTx.at(it) > 0){
            overallACThroughput.at(it) = (successfulTx.at(it) * L * 8.)/SimTime();
        }else
        {
            overallACThroughput.at(it) = 0.0;
        }
        cout << "\t+ Throughput for AC " << it << ": " << overallACThroughput.at(it) << endl;
        
        overallSxTx += successfulTx.at(it);
        cout << "\t- Time between successful transmissions for AC " << it << ": " << 
        accumTimeBetweenSxTx.at(it) / successfulTx.at(it) << endl;

        switch(it)
        {
            case 0:
                    packetsInQueue.at(it) += MACQueueBE.QueueSize();
                    cout << "\t* Queue AC 0: " << packetsInQueue.at(it) << endl;
                    break;
            case 1:
                    packetsInQueue.at(it) += MACQueueBK.QueueSize();
                    cout << "\t* Queue AC 1: " << packetsInQueue.at(it) << endl;
                    break;
            case 2:
                    packetsInQueue.at(it) += MACQueueVI.QueueSize();
                    cout << "\t* Queue AC 2: " << packetsInQueue.at(it) << endl; 
                    break;
            case 3:
                    packetsInQueue.at(it) += MACQueueVO.QueueSize();
                    cout << "\t* Queue AC 3: " << packetsInQueue.at(it) << endl;
                    break;
            default:
                    break;

        }

        totalCollisions += totalACCollisions.at(it);
        totalInternalCol += totalInternalACCol.at(it);
        cout << "\t- Total Collisions for AC " << it << ": " << totalACCollisions.at(it) << endl;
        cout << "\t\t- Total Internal Collisions for AC " << it << ": " << totalInternalACCol.at(it) << endl;

        totalRetransmissions += totalACRet.at(it);
        cout << "\t- Total Retransmissions for AC " << it << ": " << totalACRet.at(it) << endl;

        totalDropped += droppedAC.at(it);
        cout << "\t- Total Dropped due to retransmissions for AC " << it << ": " << droppedAC.at(it) << endl;

    }
    cout << "+ Overall successful transmissions: " << overallSxTx << endl;
    
    overallThroughput = (overallSxTx * L * 8.) / SimTime();
    cout << "+ Overall throughput for this station: " << overallThroughput << endl;

    cout << "- Overal Collisions for this station: " << totalCollisions << endl;

    cout << "- Overall retransmissions: " << totalRetransmissions << endl;

    cout << "- Overall dropped due to RET: " << totalDropped << endl;


    double erased = 0.0;
    double remaining = 0.0;
    for(int i = 0; i < AC; i++)
    {
        erased += ( droppedAC.at(i) + successfulTx.at(i) + backlogged.at(i) );
        remaining += ( packetsInQueue.at(i) + blockedPackets.at(i) );
    }
    cout << "- Erased packets failure index (should be 1): " << ( (incommingPackets - erased) / remaining ) << endl;
    
    
};

void STA :: in_slot(SLOT_notification &slot)
{	
	switch(slot.status)
	{
		case 0:
			//Important to remember: 0 = BE, 1 = BK, 2 = VI, 3 = VO
			for(int i = 0; i < backlogged.size(); i++)
			{
                //cout << "(" << SimTime() << ") STA-" << node_id << " AC " << i << ". Counter: " << backoffCounters.at(i)
                //    << ". Stage: " << backoffStages.at(i) << endl;

				if((backlogged.at(i) == 1) && (backoffCounters.at(i) > 0)) //if the AC has something to transmit
				{
					backoffCounters.at(i)--;
                    //cout << "STA-" << node_id << ": AC: " << i << ". backoff: " << backoffCounters.at(i) << endl;
				}
			}
			break;
		case 1:
            if(transmitted == 1)
            {
                for (int i = 0; i < backoffCounters.size(); i++)
                {
				    if( (packet.accessCategory == i) && (backoffCounters.at(i) == 0) )//this category transmitted the last packet
				    {
                        //Gathering statistics from last transmission
                        successfulTx.at(i) += superPacket.at(i).aggregation;
                        sx = 1;
                        accumTimeBetweenSxTx.at(i) += double(SimTime() - superPacket.at(i).contention_time);

                        //cout << "STA-" << node_id << ": AC: " << i << ". Transmitted" << endl;

                        //Erasing the packet(s) that was(were) sent
                        //if(packet.accessCategory == 3) cout << MACQueueVO.QueueSize() << endl;
                        erasePacketsFromQueue(MACQueueBK, MACQueueBE, MACQueueVI, MACQueueVO, superPacket.at(i), 
                            node_id, backlogged.at(i));
                        //if(packet.accessCategory == 3) cout << MACQueueVO.QueueSize() << endl;
                    
                        //If there is another packet waiting in the transmission queue, pick it and start contention

                        /*****NEW PACKET IS PICKED************
                        **************************************/
                        stationStickiness.at(i) = system_stickiness;            //Resetting the stickiness after a successful transmission
                        if(system_stickiness == 0) backoffStages.at(i) = 0;     //Resetting the backoffstage of the transmitting AC
                        retAttemptAC.at(i) = 0;                                 //Resetting the retransmission attempt counter
                        transmitted = 0;                                        //Also resetting the transmitted flag

                        //cout << "+++Tx" << endl;
                        computeBackoff(backlogged.at(i), queuesSizes.at(i), i, stationStickiness.at(i), 
                            backoffStages.at(i), backoffCounters.at(i), system_stickiness, node_id, sx, EDCA);
                        if(backlogged.at(i) > 0) pickNewPacket(i, SimTime(), superPacket, MACQueueBK, MACQueueBE, 
                            MACQueueVI, MACQueueVO, node_id);
                        break;
				    }
                }
			}else
            {
                for(int i = 0; i < backlogged.size(); i++)
                {
                    if((backlogged.at(i) == 1) && (backoffCounters.at(i) > 0)) //if the AC has something to transmit
                    {
                        backoffCounters.at(i)--;
                        //cout << "STA-" << node_id << ": AC: " << i << ". backoff: " << backoffCounters.at(i) << endl;
                    }
                }
                break;
            }
        default: //it is set to default to mean a number other than 0 or 1
            if (transmitted == 1)
            {
                for (int i = 0; i < backoffCounters.size(); i++)
                {
                    if( (packet.accessCategory == i) && (backoffCounters.at(i) == 0) )//to modify the colliding AC backoff parameters
                    {
                        //Collision metrics
                        totalACCollisions.at(i)++;
                        //Retransmission metrics
                        totalACRet.at(i)++;
                        if(retAttemptAC.at(i) == MAX_RET)
                        {
                            erasePacketsFromQueue(MACQueueBK, MACQueueBE, MACQueueVI, MACQueueVO, superPacket.at(i), 
                                node_id, backlogged.at(i));
                            droppedAC.at(i)++;
                            stationStickiness.at(i) = system_stickiness;
                            //JUST FOR EDCA
                            backoffStages.at(i) = 0;
                            //JUST FOR EDCA

                            if(backlogged.at(i) > 0) pickNewPacket(i, SimTime(), 
                                superPacket, MACQueueBK, MACQueueBE, MACQueueVI, MACQueueVO, node_id);

                            retAttemptAC.at(i) = 0;     //Resetting the retransmission attempt counter

                        }else
                        {
                            //cout << "(" << SimTime() <<") ---Station " << node_id << ": AC " << ACToTx << " collided." << endl;
                            stationStickiness.at(i) = max((stationStickiness.at(i) - 1), 0);
                            backoffStages.at(i) = min((backoffStages.at(i) + 1), MAXSTAGE);
                            retAttemptAC.at(i)++;
                        }
                        //cout << "Node " << node_id << "queue size after collision: " << MACQueueVI.QueueSize() << endl;
                        sx = 0;
                        //cout << "--Tx" << endl;
                        computeBackoff(backlogged.at(i), queuesSizes.at(i), i, stationStickiness.at(i), 
                            backoffStages.at(i), backoffCounters.at(i), system_stickiness, node_id, sx, EDCA);
                        transmitted = 0;
                        break;
                    }
                }
                
            }else
            {
                for(int i = 0; i < backlogged.size(); i++)
                {
                    if((backlogged.at(i) == 1) && (backoffCounters.at(i) > 0)) //if the AC has something to transmit
                    {
                        backoffCounters.at(i)--;
                        //cout << "STA-" << node_id << ": AC: " << i << ". backoff: " << backoffCounters.at(i) << endl;
                    }
                }
                break;
            }
	}
	
	//**********************************************
	//****Checking availability for transmission****
    //**********************************************
    //cout << "STA-" << node_id << endl;

    ACToTx = resolveInternalCollision(backlogged, queuesSizes, stationStickiness, 
        backoffStages, backoffCounters, system_stickiness, node_id, totalInternalACCol, retAttemptAC, 
        SimTime(), EDCA);

    //Fix any dropping of packets due to internal collisions
    for(int i = 0; i < retAttemptAC.size(); i++)
    {
        if(retAttemptAC.at(i) == MAX_RET)
        {
            erasePacketsFromQueue(MACQueueBK, MACQueueBE, MACQueueVI, MACQueueVO, superPacket.at(i), 
                node_id, backlogged.at(i));
            droppedAC.at(i)++;
            stationStickiness.at(i) = system_stickiness;
            //JUST FOR EDCA
            backoffStages.at(i) = 0;
            //JUST FOR EDCA

            if(backlogged.at(i) > 0) pickNewPacket(i, SimTime(), superPacket, MACQueueBK, MACQueueBE, 
                MACQueueVI, MACQueueVO, node_id);
            computeBackoff(backlogged.at(i), queuesSizes.at(i), i, stationStickiness.at(i), backoffStages.at(i), 
                backoffCounters.at(i), system_stickiness, node_id, sx, EDCA);

            retAttemptAC.at(i) = 0;     //Resetting the retransmission attempt counter

        }

    }

    //Attempting transmission if any available
    if(ACToTx >= 0){
        //cout << "(" << SimTime() << ") +++Station: " << node_id << ": transmitted AC: " << ACToTx << endl;
        packet = preparePacketForTransmission(ACToTx, SimTime(), superPacket, node_id);
        transmissions.at(ACToTx)++;
        transmitted = 1;
        out_packet(packet);
    }
    

};

void STA :: in_packet(Packet &packet)
{
    incommingPackets++;

    backlogged.at(packet.accessCategory) = 1;
    FIFO <Packet> *Q;

    //cout << "STA " << node_id << ": received a packet for category " << packet.accessCategory << endl;

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
    
    if(Q->QueueSize() < K)
    {
        Q->PutPacket(packet);
    	queuesSizes.at(packet.accessCategory) = (int)Q->QueueSize();
    }else
    {
        blockedPackets.at(packet.accessCategory)++;
    }
}

