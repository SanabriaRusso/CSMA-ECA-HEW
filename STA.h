#include <time.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <math.h>
#include <algorithm>
#include <string>
#include <array>
#include <map>
#include <bitset>
#include "Aux.h"
#include "FIFO.h"
#include "includes/computeBackoff.hh"
#include "includes/computeBackoff_enhanced.hh"
#include "includes/selectMACProtocol.hh"
#include "includes/resolveInternalCollision.hh"
#include "includes/preparePacketForTransmission.hh"
#include "includes/erasePacketsFromQueue.hh"
#include "includes/pickNewPacket.hh"
#include "includes/decrement.hh"
#include "includes/setAIFS.hh"
#include "includes/analiseHalvingCycle.hh"
#include "includes/analiseResetCycle.hh"
#include "includes/analiseBetterReset.hh"
#include "includes/dumpStationLog.hh"
#include "includes/getPayloadForTxDuration.hh"


//Suggested value is MAXSTAGE+1
#define MAX_RET 6

//Number of access categories
#define AC 4

// #define MAXSTAGE 5
extern "C" const int MAXSTAGE_ECA [AC] = { 5, 5, 5, 5 };
extern "C" const int MAXSTAGE_EDCA [AC] = { 5, 5, 1, 1 };
extern "C" const int ECA_AIFS [AC] = { 0, 0, 0, 0 };
// extern "C" const int defaultAIFS [AC] = { 0, 0, 0, 0 };
extern "C" const int defaultAIFS [AC] = { 7, 3, 2, 2 };


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
        int nodesInSim;
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
        double overallPacketsInQueue;
        std::array<double,AC> packetsInQueue;
        std::array<double,AC> queueEmpties;
        double erased;
        double remaining;


        //Transmissions statistics
        std::array<double,AC> transmissions;
        double totalTransmissions;
        std::array<double,AC> sxTx;
        std::array<double,AC> consecutiveSx;
        std::array<double,AC> packetsSent; //successfully sent packets per AC
        std::array<double,AC> bitsSent; //total payload bits sent
        double bitsFromSuperPacket;
        double overallSentPackets;
        double overallSentBits;
        std::array<double,AC> overallACThroughput;
        double overallThroughput;
        int saturated;
        std::bitset< 512 > scheduleMap;

        //Collision stsatistics
        double totalCollisions;
        std::array<double,AC> totalACCollisions;
        std::array<double,AC> lastCollision;

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
        std::array<double,AC> accumQueueingDelay;

        //Simulation acceleration
        int alwaysSaturated;
    
        //End of simulation statistic
        std::array<int, AC> backoffStagesFinal;

    private:
    	/*the positions in the backoff counters and stages vectors follow the 
    	ACs priorities, meaning: 0 = BK, 1 = BE, 2 = VI, 3 = VO*/
        std::array<double, AC> backoffCounters;
        std::array<int, AC> backoffStages;
        std::array<int, AC> previousStage;
        std::array<double,AC> AIFS;

        //Halving the cycle statistics
        int changingSchedule; // 1 = yes, 0 = no
        std::array<double, AC> halvingCounters;
        std::array<double, AC> analysisCounter;
        std::array<int,AC> halvingThresholds;
        std::array<int, AC> halvingAttempt;
        std::array<int, AC> shouldHalve; //1 = yes, 0 = no
        std::array<int,AC> changeStage;
        std::array<double,AC> halved; //number of times the backoff was halved
        std::array<int,AC> resetSuccessfull; //to reverse the reset done before a Tx that caused a collision

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
	selectMACProtocol(node_id, ECA, system_stickiness, cut);
    setAIFS(AIFS, ECA, defaultAIFS, ECA_AIFS);

    //--------------------IMPORTANT
    backoffScheme = 1; // 0 = oldScheme, 1 = newScheme
    changingSchedule = 1; // 0 = noScheReset, 1 = scheReset
    if(ECA == 0){
        backoffScheme = 0;
        changingSchedule = 0; // 0 = no, 1 = yes.
    }
    //-----------------------------
	
	/*Initializing variables and arrays to avoid warning regarding
	in-class initialization of non-static data members*/
	incommingPackets = 0;

    for(int i = 0; i < (int)superPacket.size(); i++)
    {
        superPacket.at(i).contention_time = SimTime();
        //Setting the default aggregation parameter for the first packet transmission
        superPacket.at(i).aggregation = 1;
        superPacket.at(i).L = L;
    }

    ACToTx = -1;
    transmitted = 0;
    sx = 0;
    totalTransmissions = 0;
    bitsFromSuperPacket = 0.0;

    for(int i = 0; i < AC; i++)
    {
        stationStickiness.at(i) = system_stickiness; //Could individual AC stickiness parameter be interesting?
        droppedAC.at(i) = 0;
        backlogged.at(i) = 0;
        backoffStages.at(i) = 0;
        previousStage.at(i) = 0;
        Queues.at(i) = Q;
        overallACThroughput.at(i) = 0.0;
        consecutiveSx.at(i) = 0;
        halvingCounters.at(i) = 0.0;
        analysisCounter.at(i) = 0.0;
        halvingThresholds.at(i) = 1;
        halvingAttempt.at(i) = 0;
        shouldHalve.at(i) = 0;
        changeStage.at(i) = 0;
        halved.at(i) = 0;
        resetSuccessfull.at(i) = 0;
        lastCollision.at(i) = 0;
        bitsSent.at(i) = 0.0;
    }
	
};

void STA :: Stop()
{
    //End of simulation statistics
    backoffStagesFinal = backoffStages;

    cout << "\n***Node-" << node_id << "**" <<endl;
    for(int it = 0; it < AC; it++)
    {
        cout << "AC " << it << endl;
        cout << "\t* Final Stickiness: " << stationStickiness.at(it) << " (system's: " << system_stickiness << ")." << endl;
        cout << "\t* Total transmission attempts for AC " << it << ": " << transmissions.at(it) << endl;
        totalTransmissions += transmissions.at(it);
        cout << "\t+ Total Packets sent for AC " << it << ": " << packetsSent.at(it) << endl;
        cout << "\t+ Total bits sent for AC " << it << ": " << bitsSent.at(it) << endl;
        
        if(packetsSent.at(it) > 0){
            overallACThroughput.at(it) = bitsSent.at(it)/SimTime();
        }
        cout << "\t+ Throughput for AC " << it << ": " << overallACThroughput.at(it) << endl;
        overallSentPackets += packetsSent.at(it);
        overallSentBits += bitsSent.at(it);

        if(sxTx.at(it) == 0) sxTx.at(it)++;   //Avoiding divisions by 0.
        //Adding the contention time for the last transmission attempt
        if(backlogged.at(it) == 1) accumTimeBetweenSxTx.at(it) += (double)(SimTime() - superPacket.at(it).contention_time);
        cout << "\t- Time between successful transmissions for AC " << it << ": " << 
        accumTimeBetweenSxTx.at(it) / sxTx.at(it) << endl;

        packetsInQueue.at(it) = Queues.at(it).QueueSize();
        overallPacketsInQueue += packetsInQueue.at(it);
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
    
    overallThroughput = overallSentBits / SimTime();
    cout << "+ Overall throughput for this station: " << overallThroughput << endl;

    cout << "- Overall Collisions for this station: " << totalCollisions << endl;

    cout << "- Overall retransmissions: " << totalRetransmissions << endl;

    cout << "- Overall incomming packets: " << incommingPackets << endl;

    cout << "- Overall dropped due to RET: " << totalDropped << endl;

    cout << "- Total Blocked packet due to full MAC queue: " << totalBloked << endl;

    double pFailureI_1 = 0;
    double pFailureI_2 = 0;

    erased = totalDropped + overallSentPackets;
    remaining = overallPacketsInQueue + totalBloked;

    pFailureI_1 = (incommingPackets - erased) / remaining;
    pFailureI_2 = incommingPackets / (erased + remaining);
    
    cout << "- Erased packets failure index (at least one should be 1). In sat: " <<
    pFailureI_1 << ". Non-sat: " << pFailureI_2 << endl;

    // cout << "***DEBUG: incommingPackets: " << incommingPackets << ", erased: " << erased << ", remaining: " << remaining << endl;
    double totalHalved = 0.0;
    for(int i = 0; i < AC; i++)
    {
        totalHalved += halved.at(i);
        accumQueueingDelay.at(i) /= (droppedAC.at(i) + packetsSent.at(i));
    }
    cout << "+ Overall times some deterministic backoff was changed : " << totalHalved << endl;
    // cout << "***DEBUG: final backoff stage" << endl;
    // for(int i = 0; i < AC; i++)
    // {
    //     cout << "\tAC " << i << ": " << backoffStages.at(i) << endl;
    // }

    cout << "+ Average delay (queuing + contention)" << endl;
    for(int i = 0; i < AC; i++){
        cout << "\tAC " << i << ": " << accumQueueingDelay.at(i) << endl;
    }



    //////////////////////////////////////////////////////////////////////
    //Dumping station information into a file for further processing
    //////////////////////////////////////////////////////////////////////
    string logName = "sta-" + to_string(node_id) + ".log";
    string routeToFile = "Results/stations/";
    routeToFile = routeToFile + logName;
    ofstream staFile;
    staFile.open(routeToFile.c_str(), ios::app);
    dumpStationLog(nodesInSim, node_id, ECA, staFile, overallThroughput, 
        totalCollisions/totalTransmissions, totalHalved, backoffStages);
    staFile.close();
    
};

void STA :: in_slot(SLOT_notification &slot)
{	

    // cout << "\nNEW SLOT STA-" << node_id << ": " << slot.status << endl;
	switch(slot.status)
	{
		case 0:
			//Important to remember: 0 = BK, 1 = BE, 2 = VI, 3 = VO
            //We first decrement the backoff of backlogged ACs
			for(int i = 0; i < AC; i++)
			{
                // cout << "(" << SimTime() << ") STA-" << node_id << " AC " << i << ". Counter: " << backoffCounters.at(i)
                //    << ". Stage: " << backoffStages.at(i) << endl;
				if(backlogged.at(i) == 1) //if the AC has something to transmit
				{
                    if(backoffCounters.at(i) > 0)
                    {
                        decrement(i, backoffCounters.at(i), AIFS.at(i), node_id, SimTime());
                        // cout << "STA-" << node_id << ": AC: " << i << ". backoff: " << backoffCounters.at(i) << endl;
                    }
				}
            }
            //Then we treat not backlogged ACs
            for(int i = 0; i < AC; i++)
            {
                if(backlogged.at(i) == 0)
                {
                    // cout << "STA-" << node_id << ": AC: " << i << ". Is not backlogged. Checking the queue: " << endl;
                    if(Queues.at(i).QueueSize() > 0)
                    {
                        // cout << "STA-" << node_id << ": AC: " << i << ". was not backlogged. New queue: " << Queues.at(i).QueueSize() << endl;
                        backlogged.at(i) = 1;

                        pickNewPacket(i, SimTime(), superPacket, Queues, node_id, backoffStages, fairShare, 
                            maxAggregation, MAXSTAGE_EDCA, MAXSTAGE_ECA, ECA);
                        
                        // cout << "STA-" << node_id << ": AC: " << i << ". Was not backlogged. picking a new packet." << endl;

                        int forceRandom = 0;
                        sx = 0;                        
                        if(backoffScheme == 0)
                        {
                            computeBackoff(backlogged.at(i), Queues.at(i), i, forceRandom, backoffStages.at(i), 
                                backoffCounters.at(i), system_stickiness, node_id, sx, ECA, AIFS.at(i), defaultAIFS);
                        }else
                        {
                            computeBackoff_enhanced(backlogged, Queues.at(i), i, forceRandom, backoffStages, 
                                backoffCounters, system_stickiness, node_id, sx, ECA, buffer, AIFS, ECA_AIFS);
                        }
                        
                        // cout << "\tBacklog: " << backlogged.at(i) << ". Counter: " << backoffCounters.at(i) << endl;

                    }
                }
			}
			break;
		case 1:
            if(transmitted == 1)
            {
                sx = 1; //it was a successful transmission
                /* All other stations should decrement their respective counters accouting 
                 * for the extra empty slot after DIFS */
                for(int i = 0; i < AC; i++)
                {
                    if(i != packet.accessCategory){
                        // Decremeting the backoff of all other ACs
                        if(backlogged.at(i) == 1)
                        {
                            if(backoffCounters.at(i) > 0)
                            {
                                if(ECA == 0){
                                    AIFS.at(i) = defaultAIFS[i]; //Resetting AIFS for EDCA
                                    // cout << "Resetting AIFS " << (int)defaultAIFS[i] << endl;
                                }
                                decrement(i, backoffCounters.at(i), AIFS.at(i), node_id, SimTime());    
                            }
                        }
                    }
                }
                //Locating the AC that transmitted the last packet
                for (int i = 0; i < AC; i++)
                {
				    if( (packet.accessCategory == i) && (backoffCounters.at(i) == 0) )//this category transmitted the last packet
				    {
                        //Gathering statistics from last transmission
                        packetsSent.at(i) += (packet.aggregation - slot.error);
                        sxTx.at(i)++;
                        consecutiveSx.at(i)++;
                        if(resetSuccessfull.at(i) == 1) resetSuccessfull.at(i); //for reversing a halving after collisions. Here, nothing.
                        accumTimeBetweenSxTx.at(i) += double(SimTime() - superPacket.at(i).contention_time);

                        // cout << "STA-" << node_id << ": AC: " << i << ". Transmitted " <<
                        // packet.aggregation << " packets." << endl;

                        //Erasing the packet(s) that was(were) sent
                        erasePacketsFromQueue(Queues, superPacket.at(i), node_id, backlogged.at(i), fairShare, 
                            sx, droppedAC.at(i), queueEmpties, slot.error, accumQueueingDelay.at(i), SimTime(), 
                            alwaysSaturated, bitsSent.at(i), bitsFromSuperPacket, slot.affectedFrames);

                        // cout << "(" << SimTime() << ") 1) STA-" << node_id << ": AC: " << i << ". Backlog: " << backlogged.at(i) << endl;
                    
                        /*****NEW PACKET IS PICKED************
                        **************************************/
                        if(stationStickiness.at(i) <= system_stickiness)        //Avoid resetting stickiness if schedule was reduced using dynamic stickiness
                        {
                            stationStickiness.at(i) = system_stickiness;        //Resetting the stickiness after a successful transmission
                        }
                        if(ECA == 0 || system_stickiness == 0) backoffStages.at(i) = 0;                   //Resetting the backoffstage of the transmitting AC
                        retAttemptAC.at(i) = 0;                                 //Resetting the retransmission attempt counter
                        transmitted = 0;                                        //Also resetting the transmitted flag

                        // cout << "+++Tx" << endl;
                        if(backlogged.at(i) == 1)
                        {
                            pickNewPacket(i, SimTime(), superPacket, Queues, node_id, backoffStages, fairShare, 
                                maxAggregation, MAXSTAGE_EDCA, MAXSTAGE_ECA, ECA);
                            // cout << "\nSTA-" << node_id << ": Success AC " << i << " slot: " << slot.num;

                            //I can calculate the backoff freely here because it was a successful transmissions
                            //where the backoff is deterministic and no internal collision is possible after a SmartBackoff
                            if(backoffScheme == 0)
                            {
                                computeBackoff(backlogged.at(i), Queues.at(i), i, stationStickiness.at(i), backoffStages.at(i), 
                                    backoffCounters.at(i), system_stickiness, node_id, sx, ECA, AIFS.at(i), defaultAIFS);
                            }else
                            {
                                computeBackoff_enhanced(backlogged, Queues.at(i), i, stationStickiness.at(i), backoffStages, 
                                    backoffCounters, system_stickiness, node_id, sx, ECA, buffer, AIFS, ECA_AIFS);
                            }
                            // cout << ". Counter: " << backoffCounters.at(i) << endl;
                        }else
                        {
                            backoffStages.at(i) = 0;
                            backoffCounters.at(i) = 0;
                            stationStickiness.at(i) = system_stickiness;
                        }
				    }
                }
			}else
            {
                for(int i = 0; i < AC; i++)
                {
                    // Decremeting the backoff of all ACs
                    if(backoffCounters.at(i) > 0)
                    {
                        if(ECA == 0){
                                AIFS.at(i) = defaultAIFS[i]; //Resetting AIFS for EDCA
                                // cout << "Resetting AIFS " << (int)defaultAIFS[i] << endl;
                        }
                        decrement(i, backoffCounters.at(i), AIFS.at(i), node_id, SimTime());
                    }
                }
            }
            break;

        default: //it is set to default to mean a number other than 0 or 1
            if (transmitted == 1)
            {
                sx = 0; //it was not a successful transmission

                for(int i = 0; i < AC; i++)
                {
                    if( (packet.accessCategory != i) && (backoffCounters.at(i) > 0) )
                    {
                        if(ECA == 0){
                                AIFS.at(i) = defaultAIFS[i]; //Resetting AIFS for EDCA
                                // cout << "Resetting AIFS " << (int)defaultAIFS[i] << endl;
                        }
                        decrement(i, backoffCounters.at(i), AIFS.at(i), node_id, SimTime());
                    }
                }

                for (int i = 0; i < AC; i++)
                {
                    if( (packet.accessCategory == i) && (backoffCounters.at(i) == 0) )//to modify the colliding AC(s) backoff parameters
                    {
                        //Collision metrics
                        totalACCollisions.at(i)++;
                        lastCollision.at(i) = SimTime();

                        //Retransmission metrics
                        totalACRet.at(i)++;
                        retAttemptAC.at(i)++;

                        if(retAttemptAC.at(i) == MAX_RET)
                        {
                            // cout << "Station " << node_id << ": AC " << ACToTx << ". Drops " 
                            // << (int) pow(2, superPacket.at(i).startContentionStage) << " packets for collisions." << endl;

                            //Adding the already elapsed time to the timeBetweenSxTx
                            accumTimeBetweenSxTx.at(i) += double(SimTime() - superPacket.at(i).contention_time);

                            erasePacketsFromQueue(Queues, superPacket.at(i), node_id, backlogged.at(i), 
                                fairShare, sx, droppedAC.at(i), queueEmpties, slot.error, accumQueueingDelay.at(i), SimTime(), 
                                alwaysSaturated, bitsSent.at(i), bitsFromSuperPacket, slot.affectedFrames);

                            stationStickiness.at(i) = system_stickiness;

                            if(ECA == 0 || ECA == 3) backoffStages.at(i) = 0;

                            if(backlogged.at(i) == 1)
                            {
                                pickNewPacket(i, SimTime(), superPacket, Queues, node_id, backoffStages, fairShare, 
                                    maxAggregation, MAXSTAGE_EDCA, MAXSTAGE_ECA, ECA);
                            }else
                            {
                                backoffStages.at(i) = 0;
                                backoffCounters.at(i) = 0;
                                stationStickiness.at(i) = system_stickiness;
                            }

                            retAttemptAC.at(i) = 0;     //Resetting the retransmission attempt counter

                        }else
                        {
                            stationStickiness.at(i) = max( (stationStickiness.at(i) - 1), 0 );
                            if(stationStickiness.at(i) == 0) //subjecting the halving statistics to the level of stickiness
                            {
                                //----------------------------
                                //Reversing a halving
                                if(resetSuccessfull.at(i) == 1)
                                {
                                        int maxStage = MAXSTAGE_ECA[i];
                                        if(ECA == 0)
                                        {
                                            maxStage = MAXSTAGE_EDCA[i];
                                        }

                                        backoffStages.at(i) = min( previousStage.at(i), maxStage );
                                        resetSuccessfull.at(i) = 0;
                                }
                                //----------------------------
                                // cout << "(" << SimTime() <<") ---Station " << node_id << ": AC " << ACToTx << " collided. (slot " << slot.num << ")" << endl;
                                consecutiveSx.at(i) = 0;
                                halvingAttempt.at(i) = 0;
                                int maxStage = MAXSTAGE_ECA[i];
                                if(ECA == 0)
                                {
                                    maxStage = MAXSTAGE_EDCA[i];
                                }

                                backoffStages.at(i) = min( (backoffStages.at(i) + 1), maxStage );
                            }
                        }
                        //cout << "Node " << node_id << "queue size after collision: " << MACQueueVI.QueueSize() << endl;
                        //cout << "--Tx" << endl;

                        // cout << "\nSTA-" << node_id << ": Collision" << endl;

                        if(backoffScheme == 0)
                        {
                            computeBackoff(backlogged.at(i), Queues.at(i), i, stationStickiness.at(i), backoffStages.at(i), 
                                backoffCounters.at(i), system_stickiness, node_id, sx, ECA, AIFS.at(i), defaultAIFS);
                        }else
                        {
                            computeBackoff_enhanced(backlogged, Queues.at(i), i, stationStickiness.at(i), backoffStages, 
                                backoffCounters, system_stickiness, node_id, sx, ECA, buffer, AIFS, ECA_AIFS);
                        }

                        // cout << "(" << SimTime() <<") ---Station " << node_id << ": AC " << ACToTx << " new backoff: " << backoffCounters.at(ACToTx) << ". (slot " << slot.num << ")" << endl;
                        transmitted = 0;
                    }
                }  
            }else
            {
                for(int i = 0; i < AC; i++)
                {
                    // Decremeting the backoff of all other ACs
                    if(backoffCounters.at(i) > 0)
                    {
                        if(ECA == 0){
                                AIFS.at(i) = defaultAIFS[i]; //Resetting AIFS for EDCA
                                // cout << "Resetting AIFS " << (int)defaultAIFS[i] << endl;
                        }
                        decrement(i, backoffCounters.at(i), AIFS.at(i), node_id, SimTime());
                    }
                }
            }
	}
	
	//**********************************************
	//****Checking availability for transmission****
    //**********************************************
    ACToTx = resolveInternalCollision(backoffCounters, backlogged, stationStickiness, backoffStages, 
        recomputeBackoff, totalInternalACCol, retAttemptAC, backoffScheme, node_id, MAXSTAGE_ECA, MAXSTAGE_EDCA, 
        consecutiveSx, SimTime());


    //Fix any dropping of packets due to internal collisions
    //This should not happen in CSMA/ECAqos
    if(ACToTx >= 0)
    {
        for(int i = 0; i < ACToTx; i++)
        {
            if(backlogged.at(i) == 1)
            {
                if(retAttemptAC.at(i) == MAX_RET)
                {
                    // cout << "Station " << node_id << ": AC " << ACToTx << ". Drops " 
                    // << (int) pow(2, superPacket.at(i).startContentionStage) << " packets for internal collisions." << endl;

                    //Adding the already elapsed time to the timeBetweenSxTx
                    accumTimeBetweenSxTx.at(i) += double(SimTime() - superPacket.at(i).contention_time);

                    erasePacketsFromQueue(Queues, superPacket.at(i), node_id, backlogged.at(i), 
                        fairShare, sx, droppedAC.at(i), queueEmpties, slot.error, accumQueueingDelay.at(i), SimTime(), 
                        alwaysSaturated, bitsSent.at(i), bitsFromSuperPacket, slot.affectedFrames);

                    stationStickiness.at(i) = system_stickiness;

                    if(ECA == 0) backoffStages.at(i) = 0;

                    if(backlogged.at(i) == 1)
                    {
                        pickNewPacket(i, SimTime(), superPacket, Queues, node_id, backoffStages, fairShare, maxAggregation, 
                            MAXSTAGE_EDCA, MAXSTAGE_ECA, ECA);
                    }else
                    {
                        backoffStages.at(i) = 0;
                        backoffCounters.at(i) = 0;
                        stationStickiness.at(i) = system_stickiness;
                        recomputeBackoff.at(i) = 0; //we won't recompute the backoff of not backlogged stations
                    }
                }
                retAttemptAC.at(i) = 0;     //Resetting the retransmission attempt counter
            }
        }
        
        //*****Recomputing the backoff for**
        //*****internal collisions**********
        for(int i = 0; i < ACToTx; i++)
        {
            if(recomputeBackoff.at(i) == 1)
            {
                sx = 0;
                // cout << "\nSTA-" << node_id << ": Recomputing backoff for AC " << i << endl;

                //Forcing the computation to derive a random backoff by setting the stickiness of the AC to 0.
                int forceRandom = 0;
                if(backoffScheme == 0)
                {
                    computeBackoff(backlogged.at(i), Queues.at(i), i, forceRandom, backoffStages.at(i), 
                        backoffCounters.at(i), system_stickiness, node_id, sx, ECA, AIFS.at(i), defaultAIFS);
                }else
                {
                    computeBackoff_enhanced(backlogged, Queues.at(i), i, forceRandom, backoffStages, 
                        backoffCounters, system_stickiness, node_id, sx, ECA, buffer, AIFS, ECA_AIFS);
                }
            }
        }

        //Attempting transmission if any available
        // cout << "Winner " << ACToTx << endl;

        if(backoffCounters.at(ACToTx) == 0)
        {
            packet = preparePacketForTransmission(ACToTx, SimTime(), superPacket, node_id, backoffStages, Queues, fairShare, ECA);
            packet.L = getPayloadForTxDuration(packet, Queues);
            // cout << "(" << SimTime() << ") +++Station: " << node_id << ": will transmit AC " << ACToTx
            // << ". " << packet.aggregation << " packets, " << "payload: " << packet.L << endl;
            transmissions.at(ACToTx)++;
            bitsFromSuperPacket = packet.L;
            transmitted = 1;
            out_packet(packet);
        }
    }

    //Checking if it is possible to halve the cycle length for this station
    //Limiting it to ECA with hysteresis only
    if( (changingSchedule == 1) && (ECA == 1) && (system_stickiness > 0) )
    {
        // analiseHalvingCycle(consecutiveSx, halvingCounters, backoffStages, backoffCounters, ACToTx,
        //     MAXSTAGE, backlogged, halvingAttempt, slot.status, shouldHalve, halvingThresholds, node_id, 
        //     changeStage, halved, stationStickiness, system_stickiness);

        // analiseResetCycle(consecutiveSx, halvingCounters, backoffStages, backoffCounters, ACToTx,
        //     MAXSTAGE_ECA, backlogged, halvingAttempt, slot, shouldHalve, halvingThresholds, node_id, 
        //     changeStage, halved, stationStickiness, system_stickiness, analysisCounter, SimTime());

        analiseBetterReset(consecutiveSx, halvingCounters, backoffStages, backoffCounters, ACToTx,
        MAXSTAGE_ECA, backlogged, halvingAttempt, slot, shouldHalve, halvingThresholds, node_id, 
        changeStage, halved, stationStickiness, system_stickiness, analysisCounter, SimTime(), 
        scheduleMap, resetSuccessfull, previousStage);
    }
    

};

void STA :: in_packet(Packet &packet)
{
    incommingPackets++;
    if( Queues.at(packet.accessCategory).QueueSize()  < K )
    {
        packet.queuing_time = SimTime();
        Queues.at(packet.accessCategory).PutPacket(packet);
        // cout << "Arriving, Ac-" << packet.accessCategory << ": Seq: " << packet.seq << ": Load: " << packet.L << endl;
    }else
    {
        blockedPackets.at(packet.accessCategory)++;
    }
}

