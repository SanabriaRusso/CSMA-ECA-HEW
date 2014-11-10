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
        int hysteresis;
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

        //Transmissions statistics
        std::array<double,AC> transmissions = {};
        std::array<double,AC> successfulTx = {}; //successful transmissions per AC

    private:
    	/*the positions in the backoff counters and stages vectors follow the 
    	ACs priorities, meaning: 0 = BE, 1 = BK, 2 = VI, 3 = VO*/
        std::array<double, AC> backoffCounters = {};
        std::array<int, AC> backoffStages = {};
        
        /*For better understanding, the ACs are defined as constants for navigating
        the arrays*/
        int BE; //Best-effort
        int BK; //Background
        int VI; //Video
        int VO; //Voice
        int ACToTx; //to dertermine which AC is to transmit in case of an internal collision
        
        std::array<int,AC> backlogged = {}; //whether or not the station has something to transmit on an AC (0 no, 1 yes)

		Packet packet; //individual packets
        std::array<Packet,AC> superPacket = {}; //an abstract packet composed of #AC packets. This structure helps at keeping track of AC-related timers
        
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

	for(int i = 0; i < AC; i++)
	{
		computeBackoff(backlogged.at(i), queuesSizes.at(i), i, stationStickiness.at(i), backoffStages.at(i), backoffCounters.at(i));
    }

};

void STA :: Start()
{
	selectMACProtocol(cut, node_id, EDCA, hysteresis, fairShare, maxAggregation);
	
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
    ACToTx = 0;
	
};

void STA :: Stop()
{

    for(auto it = successfulTx.begin(); it != successfulTx.end(); it++)
    {
        cout << "Total successfulTx for AC " << std::distance(successfulTx.begin(),it) << ": " 
        << *it << endl;
    }

    /*cout << "Debug Queue" << endl;
    cout << "Node #" << node_id << endl;
    cout << "\tAC 0: " << MACQueueBE.QueueSize() << endl;
    cout << "\tAC 1: " << MACQueueBK.QueueSize() << endl;
    cout << "\tAC 2: " << MACQueueVI.QueueSize() << endl;
    cout << "\tAC 3: " << MACQueueVO.QueueSize() << endl << endl;*/
    
    
};

void STA :: in_slot(SLOT_notification &slot)
{	
	switch(slot.status)
	{
		case 0:
			//Important to remember: 0 = BE, 1 = BK, 2 = VI, 3 = VO
			for(int i = 0; i < backlogged.size(); i++)
			{
				if(backlogged.at(i) == 0) //if the AC is not backlogged
				{
                    //Attempting to generate backoff counter if any packet arrives at the AC queue
					computeBackoff(backlogged.at(i), queuesSizes.at(i), i, stationStickiness.at(i), backoffStages.at(i), backoffCounters.at(i));
				}else //if the AC has something to transmit
				{
					if(backoffCounters.at(i) > 0)
					{
						backoffCounters.at(i)--;
					}
				}
			}
			break;
		case 1:
			for (int i = 0; i < backoffCounters.size(); i++)
			{
				if( (backoffCounters.at(i) == 0) && (packet.accessCategory == ACToTx) )//this category transmitted the last packet
				{
                    //Gathering statistics from last transmission
                    successfulTx.at(i) += superPacket.at(i).aggregation;

                    //Erasing the packet(s) that was(were) sent
                    erasePacketsFromQueue(MACQueueBK, MACQueueBE, MACQueueVI, MACQueueVO, superPacket.at(i));
                    
                    //If there is another packet waiting in the transmission queue, pick it and start contention

                    /*****NEW PACKET IS PICKED************
                    **************************************/
					computeBackoff(backlogged.at(i), queuesSizes.at(i), i, stationStickiness.at(i), backoffStages.at(i), backoffCounters.at(i));
                    if(backlogged.at(i) > 0) pickNewPacket(i, SimTime(), superPacket, MACQueueBK, MACQueueBE, MACQueueVI, MACQueueVO);
                    break;
				}
			}
            break;
        default: //it is set to default to mean a number other than 0 or 1
            for (int i = 0; i < backoffCounters.size(); i++)
            {
                if (backoffCounters.at(i) == 0) //to modify the colliding AC backoff parameters
                {
                    stationStickiness.at(i) = max((stationStickiness.at(i) - 1), 0);
                    backoffStages.at(i) = min(backoffStages.at(i) + 1, MAXSTAGE);
                    computeBackoff(backlogged.at(i), queuesSizes.at(i), i, stationStickiness.at(i), backoffStages.at(i), backoffCounters.at(i));

                }
            }
	}
	
	//**********************************************
	//****Checking availability for transmission****
    //**********************************************
    //cout << "STA-" << node_id << endl;

    ACToTx = resolveInternalCollision(backlogged, queuesSizes, stationStickiness, backoffStages, backoffCounters);
    
    if(ACToTx >= 0){
        packet = preparePacketForTransmission(ACToTx, SimTime(), superPacket);
        transmissions.at(ACToTx)++;
        out_packet(packet);
    }
    

};

void STA :: in_packet(Packet &packet)
{
    incommingPackets++;
    
    switch (packet.accessCategory){
    	case 0:
    		if(MACQueueBE.QueueSize() < K)
    		{
    			MACQueueBE.PutPacket(packet);
    			queuesSizes.at(packet.accessCategory) = MACQueueBE.QueueSize();
    		}else
    		{
    			blockedPackets.at(BE)++;
    		}
    		break;
    	case 1:
    		if(MACQueueBK.QueueSize() < K)
    		{
    			MACQueueBK.PutPacket(packet);
    			queuesSizes.at(packet.accessCategory) = MACQueueBK.QueueSize();
    		}else
    		{
    			blockedPackets.at(BK)++;
    		}
    		break;
    	case 2:
    		if(MACQueueVI.QueueSize() < K)
    		{
    			MACQueueVI.PutPacket(packet);
    			queuesSizes.at(packet.accessCategory) = MACQueueVI.QueueSize();
    		}else
    		{
    			blockedPackets.at(VI)++;
    		}
    		break;
    	case 3:
    		if(MACQueueVO.QueueSize() < K)
    		{
    			MACQueueVO.PutPacket(packet);
    			queuesSizes.at(packet.accessCategory) = MACQueueVI.QueueSize();
    		}else
    		{
    			blockedPackets.at(VO)++;
    		}
    		break;
    }
}

