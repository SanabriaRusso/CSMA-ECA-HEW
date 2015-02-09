#include <time.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <math.h>
#include <algorithm>
#include <array>
#include <map>
#include "Aux.h"
#include "FIFO.h"
#include "includes/computeBackoff.hh"
#include "includes/computeBackoff_enhanced.hh"
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
        int ECA;
        int backoffScheme;

        //Performance enhancement variables
        std::map<double,double> buffer;
        
        //aggregation settings
        int maxAggregation;
        int L; //length of packet
        
        //Source and Queue management
        double incommingPackets;
        std::array<double,AC> blockedPackets;
        double totalBloked;
        std::array<double,AC> queuesSizes;
        std::array<double,AC> packetsInQueue;
        std::array<double,AC> queueEmpties;
        double erased;
        double remaining;


        //Transmissions statistics
        std::array<double,AC> transmissions;
        std::array<double,AC> sxTx;
        std::array<double,AC> packetsSent; //successfully sent packets per AC
        double overallSentPackets;
        std::array<double,AC> overallACThroughput;
        double overallThroughput;

        //Collision stsatistics
        double totalCollisions;
        std::array<double,AC> totalACCollisions;

        double totalInternalCol;
        std::array<double,AC> totalInternalACCol;
        std::array<int,AC> recomputeBackoff;

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

        int ACToTx; //to dertermine which AC is to transmit in case of an internal collision
        
        std::array<int,AC> backlogged; //whether or not the station has something to transmit on an AC (0 no, 1 yes)

		Packet packet; //individual packets
        std::array<Packet,AC> superPacket; //an abstract packet composed of #AC packets. This structure helps at keeping track of AC-related timers
        
        std::array<FIFO <Packet>, AC> Queues;
        FIFO <Packet> Q;

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
	selectMACProtocol(node_id, ECA, system_stickiness);

    backoffScheme = 1; // 0 = oldScheme, 1 = newScheme

    //cout << ECA << endl;
	
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

    ACToTx = -1;
    transmitted = 0;
    sx = 0;

    for(int i = 0; i < AC; i++)
    {
        stationStickiness.at(i) = system_stickiness; //Could individual AC stickiness parameter be interesting?
        droppedAC.at(i) = 0;
        backlogged.at(i) = 0;
        backoffStages.at(i) = 0;
        Queues.at(i) = Q;
        overallACThroughput.at(i) = 0.0;
        
        if(backoffScheme == 0)
        {
            computeBackoff(backlogged.at(i), Queues.at(i), i, stationStickiness.at(i), 
                backoffStages.at(i), backoffCounters.at(i), system_stickiness, node_id, sx, ECA);
        }else
        {
            computeBackoff_enhanced(backlogged, Queues.at(i), i, stationStickiness.at(i), 
                backoffStages, backoffCounters, system_stickiness, node_id, sx, ECA, buffer);
        }
    }
	
};

void STA :: Stop()
{
    cout << "\n***Node-" << node_id << "**" <<endl;
    for(int it = 0; it < AC; it++)
    {
        cout << "AC " << it << endl;
        cout << "\t* Final Stickiness: " << stationStickiness.at(it) << " (system's: " << system_stickiness << ")." << endl;
        cout << "\t* Total transmission attempts for AC " << it << ": " << transmissions.at(it) << endl;
        cout << "\t+ Total Packets sent for AC " << it << ": " << packetsSent.at(it) << endl;
        
        if(packetsSent.at(it) > 0){
            overallACThroughput.at(it) = (packetsSent.at(it) * L * 8.)/SimTime();
        }else
        {
            overallACThroughput.at(it) = 0.0;
        }
        cout << "\t+ Throughput for AC " << it << ": " << overallACThroughput.at(it) << endl;
        
        overallSentPackets += packetsSent.at(it);
        cout << "\t- Time between successful transmissions for AC " << it << ": " << 
        accumTimeBetweenSxTx.at(it) / sxTx.at(it) << endl;

        packetsInQueue.at(it) = Queues.at(it).QueueSize();
        cout << "\t* Queue AC " << it << ": " << packetsInQueue.at(it) << endl;
        cout << "\t* Number of queue flushes for AC " << it << ": " << queueEmpties.at(it) << endl;

        totalCollisions += totalACCollisions.at(it);
        totalInternalCol += totalInternalACCol.at(it);
        cout << "\t- Total Collisions for AC " << it << ": " << totalACCollisions.at(it) << endl;
        cout << "\t\t- Total Internal Collisions for AC " << it << ": " << totalInternalACCol.at(it) << endl;

        totalRetransmissions += totalACRet.at(it);
        cout << "\t- Total Retransmissions for AC " << it << ": " << totalACRet.at(it) << endl;

        totalDropped += droppedAC.at(it);
        cout << "\t- Total Dropped due to retransmissions for AC " << it << ": " << droppedAC.at(it) << endl;

        totalBloked += blockedPackets.at(it);
        cout << "\t- Total Blocked due to full MAC queue for AC " << it << ": " << blockedPackets.at(it) << endl;

    }
    cout << "+ Overall successful transmissions: " << overallSentPackets << endl;
    
    overallThroughput = (overallSentPackets * L * 8.) / SimTime();
    cout << "+ Overall throughput for this station: " << overallThroughput << endl;

    cout << "- Overal Collisions for this station: " << totalCollisions << endl;

    cout << "- Overall retransmissions: " << totalRetransmissions << endl;

    cout << "- Overall incomming packets: " << incommingPackets << endl;

    cout << "- Overall dropped due to RET: " << totalDropped << endl;

    cout << "- Total Blocked packet due to full MAC queue: " << totalBloked << endl;

    double pFailureI_1 = 0.0;
    double pFailureI_2 = 0.0;

    for(int i = 0; i < AC; i++)
    {
        erased += ( droppedAC.at(i) + packetsSent.at(i) );
        remaining += ( packetsInQueue.at(i) + blockedPackets.at(i) );
    }

    pFailureI_1 = (incommingPackets - erased) / remaining;
    pFailureI_2 = incommingPackets / erased;
    
    cout << "- Erased packets failure index (at least one should be 1). In sat: " <<
    pFailureI_1 << ". Non-sat: " << pFailureI_2 << endl;

    
};

void STA :: in_slot(SLOT_notification &slot)
{	
	switch(slot.status)
	{
		case 0:
			//Important to remember: 0 = BE, 1 = BK, 2 = VI, 3 = VO
			for(int i = 0; i < backlogged.size(); i++)
			{
                // cout << "(" << SimTime() << ") STA-" << node_id << " AC " << i << ". Counter: " << backoffCounters.at(i)
                //    << ". Stage: " << backoffStages.at(i) << endl;

				if((backlogged.at(i) == 1) && (backoffCounters.at(i) > 0)) //if the AC has something to transmit
				{
					backoffCounters.at(i)--;
                    // cout << "STA-" << node_id << ": AC: " << i << ". backoff: " << backoffCounters.at(i) << endl;
				}else if(backlogged.at(i) == 0)
                {
                    // cout << "STA-" << node_id << ": AC: " << i << ". Is not backlogged. Checking the queue." << endl;
                    if(Queues.at(i).QueueSize() > 0)
                    {
                        backlogged.at(i) = 1;

                        pickNewPacket(i, SimTime(), superPacket, Queues, node_id, backoffStages, fairShare);
                        // computeBackoff(backlogged.at(i), Queues.at(i), i, stationStickiness.at(i), 
                        //     backoffStages.at(i), backoffCounters.at(i), system_stickiness, node_id, sx, ECA);

                        // cout << "\nWas not backlogged" << endl;
                        int forceRandom = 0;
                        
                        if(backoffScheme == 0)
                        {
                            computeBackoff(backlogged.at(i), Queues.at(i), i, forceRandom, 
                                backoffStages.at(i), backoffCounters.at(i), system_stickiness, node_id, sx, ECA);
                        }else
                        {
                            computeBackoff_enhanced(backlogged, Queues.at(i), i, forceRandom, 
                                backoffStages, backoffCounters, system_stickiness, node_id, sx, ECA, buffer);
                        }

                        // cout << "STA-" << node_id << ": AC: " << i << ". Was not backlogged. picking a new packet." << endl;
                        // cout << "\tBacklog: " << backlogged.at(i) << ". Counter: " << backoffCounters.at(i) << endl;

                    }
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
                        packetsSent.at(i) += packet.aggregation;
                        sxTx.at(i)++;
                        sx = 1; //it was a successful transmission
                        accumTimeBetweenSxTx.at(i) += double(SimTime() - superPacket.at(i).contention_time);

                        // cout << "1) STA-" << node_id << ": AC: " << i << ". Transmitted " <<
                        // packet.aggregation << " packets." << endl;

                        //Erasing the packet(s) that was(were) sent
                        erasePacketsFromQueue(Queues, superPacket.at(i), node_id, backlogged.at(i), fairShare, 
                            sx, droppedAC.at(i), queueEmpties);

                        // cout << "(" << SimTime() << ") 1) STA-" << node_id << ": AC: " << i << ". Backlog: " << backlogged.at(i) << endl;
                    
                        //If there is another packet waiting in the transmission queue, pick it and start contention
                        /*****NEW PACKET IS PICKED************
                        **************************************/
                        stationStickiness.at(i) = system_stickiness;            //Resetting the stickiness after a successful transmission
                        if(system_stickiness == 0) backoffStages.at(i) = 0;     //Resetting the backoffstage of the transmitting AC
                        retAttemptAC.at(i) = 0;                                 //Resetting the retransmission attempt counter
                        transmitted = 0;                                        //Also resetting the transmitted flag

                        //cout << "+++Tx" << endl;
                        if(backlogged.at(i) == 1)
                        {
                            pickNewPacket(i, SimTime(), superPacket, Queues, node_id, backoffStages, fairShare);

                            
                            // cout << "\nSuccess" << endl;
                            if(backoffScheme == 0)
                            {
                                computeBackoff(backlogged.at(i), Queues.at(i), i, stationStickiness.at(i), 
                                    backoffStages.at(i), backoffCounters.at(i), system_stickiness, node_id, sx, ECA);
                            }else
                            {
                                computeBackoff_enhanced(backlogged, Queues.at(i), i, stationStickiness.at(i), 
                                    backoffStages, backoffCounters, system_stickiness, node_id, sx, ECA, buffer);
                            }
                        }

                        // Decremeting the backoff of all other ACs
                        for(int j = 0; j < AC; j++)
                        {
                            if(j != i)
                            {
                                if(backoffCounters.at(j) > 0) //if the AC has something to transmit
                                {
                                    backoffCounters.at(j)--;
                                }
                            }
                        }
				    }
                }
			}
            break;
        default: //it is set to default to mean a number other than 0 or 1
            if (transmitted == 1)
            {
                for (int i = 0; i < backoffCounters.size(); i++)
                {
                    if( (packet.accessCategory == i) && (backoffCounters.at(i) == 0) )//to modify the colliding AC backoff parameters
                    {
                        //Collision metrics
                        totalACCollisions.at(i)++;
                        sx = 0;
                        //Retransmission metrics
                        totalACRet.at(i)++;

                        if(retAttemptAC.at(i) >= MAX_RET)
                        {
                            // cout << "3) Station " << node_id << ": AC " << ACToTx << ". Drops " 
                            // << (int) pow(2, superPacket.at(i).startContentionStage) << " packets for internall collisions." << endl;

                            //Adding the already elapsed time to the timeBetweenSxTx
                            accumTimeBetweenSxTx.at(i) += double(SimTime() - superPacket.at(i).contention_time);

                            erasePacketsFromQueue(Queues, superPacket.at(i), node_id, backlogged.at(i), 
                                fairShare, sx, droppedAC.at(i), queueEmpties);

                            stationStickiness.at(i) = system_stickiness;

                            if(ECA == 0) backoffStages.at(i) = 0;
                            if(backlogged.at(i) == 1) pickNewPacket(i, SimTime(), superPacket, Queues, node_id, 
                                backoffStages, fairShare);

                            retAttemptAC.at(i) = 0;     //Resetting the retransmission attempt counter

                        }else
                        {
                            // cout << "(" << SimTime() <<") ---Station " << node_id << ": AC " << ACToTx << " collided." << endl;
                            stationStickiness.at(i) = max( (stationStickiness.at(i) - 1), 0 );
                            backoffStages.at(i) = min( (backoffStages.at(i) + 1), MAXSTAGE );
                            retAttemptAC.at(i)++;
                        }
                        //cout << "Node " << node_id << "queue size after collision: " << MACQueueVI.QueueSize() << endl;
                        //cout << "--Tx" << endl;


                        // cout << "\nCollision" << endl;

                        if(backoffScheme == 0)
                        {
                            computeBackoff(backlogged.at(i), Queues.at(i), i, stationStickiness.at(i), 
                                backoffStages.at(i), backoffCounters.at(i), system_stickiness, node_id, sx, ECA);
                        }else
                        {
                            computeBackoff_enhanced(backlogged, Queues.at(i), i, stationStickiness.at(i), 
                                backoffStages, backoffCounters, system_stickiness, node_id, sx, ECA, buffer);
                        }

                        transmitted = 0;
                    }
                }
                
            }
            break;
	}
	
	//**********************************************
	//****Checking availability for transmission****
    //**********************************************

    ACToTx = resolveInternalCollision(backoffCounters, backlogged, stationStickiness, backoffStages, 
        recomputeBackoff, totalInternalACCol, retAttemptAC);


    //*****Recomputing the backoff for**
    //*****internal collisions**********
    sx = 0;
    for(int i = 0; i < AC; i++)
    {
        if(recomputeBackoff.at(i) == 1)
        {
            // cout << "\nRecomputing backoff" << endl;

            //Forcing the computation to derive a random backoff by setting the stickiness of the AC to 0.
            int forceRandom = 0;
            if(backoffScheme == 0)
            {
                computeBackoff(backlogged.at(i), Queues.at(i), i, forceRandom, 
                    backoffStages.at(i), backoffCounters.at(i), system_stickiness, node_id, sx, ECA);
            }else
            {
                computeBackoff_enhanced(backlogged, Queues.at(i), i, forceRandom, 
                    backoffStages, backoffCounters, system_stickiness, node_id, sx, ECA, buffer);
            }
        }
    }


    //Attempting transmission if any available
    if(ACToTx >= 0){
        //Fix any dropping of packets due to internal collisions

        ////////////////////////////////
        //This should disappear soon.///
        ////////////////////////////////
        for(int i = 0; i <= ACToTx; i++)
        {
            if(retAttemptAC.at(i) >= MAX_RET)
            {
                // cout << "3) Station " << node_id << ": AC " << ACToTx << ". Drops " 
                // << (int) pow(2, superPacket.at(i).startContentionStage) << " packets for internall collisions." << endl;

                //Adding the already elapsed time to the timeBetweenSxTx
                accumTimeBetweenSxTx.at(i) += double(SimTime() - superPacket.at(i).contention_time);

                erasePacketsFromQueue(Queues, superPacket.at(i), node_id, backlogged.at(i), fairShare, 
                    sx, droppedAC.at(i), queueEmpties);
                stationStickiness.at(i) = system_stickiness;
                
                if(ECA == 0) backoffStages.at(i) = 0;
                
                if(backlogged.at(i) == 1) pickNewPacket(i, SimTime(), superPacket, Queues, node_id,
                    backoffStages, fairShare);

                // cout << "Dropping after IC" << endl;

                if(backoffScheme == 0)
                {
                    computeBackoff(backlogged.at(i), Queues.at(i), i, stationStickiness.at(i), 
                        backoffStages.at(i), backoffCounters.at(i), system_stickiness, node_id, sx, ECA);
                }else
                {
                    computeBackoff_enhanced(backlogged, Queues.at(i), i, stationStickiness.at(i), 
                        backoffStages, backoffCounters, system_stickiness, node_id, sx, ECA, buffer);
                }

                retAttemptAC.at(i) = 0;     //Resetting the retransmission attempt counter
            }

        }
        ///////////////////////////////////////
        //If we eliminate internal collisions//
        //no packet will be dropped////////////


        packet = preparePacketForTransmission(ACToTx, SimTime(), superPacket, node_id, backoffStages, Queues, fairShare);
        // cout << "(" << SimTime() << ") +++Station: " << node_id << ": will transmit AC " << ACToTx
        // << ". " << packet.aggregation << " packets." << endl;

        transmissions.at(ACToTx)++;
        transmitted = 1;
        out_packet(packet);
    }
    

};

void STA :: in_packet(Packet &packet)
{
    incommingPackets++;

    if( Queues.at(packet.accessCategory).QueueSize()  < K )
    {
        Queues.at(packet.accessCategory).PutPacket(packet);
    }else
    {
        blockedPackets.at(packet.accessCategory)++;
    }
}

