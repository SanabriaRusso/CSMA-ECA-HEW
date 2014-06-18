#include <math.h>
#include <iostream>
#include <fstream>
#include "Aux.h"
#include "FIFO.h"
#include "includes/backoff.hh"

//#define CWMIN 16 //to comply with 802.11n it should 16. Was 32 for 802.11b.
#define MAXSTAGE 5

//Suggested value is MAXSTAGE+1
#define MAX_RET 6


using namespace std;

component STA : public TypeII
{
    public:
        void Setup();
        void Start();
        void Stop();

    public:
        int node_id;
        int K; //max queue size
        int system_stickiness; //global stickiness
        int station_stickiness;
        int hysteresis;
        int fairShare;
	
        /*long int observed_slots;
        long int empty_slots;
                                                                         
        
        long int collisions;
        long int total_transmissions;
        long int successful_transmissions;
        long int droppedPackets; //due to retransmissions
        long int packetDisposal;
        long int driftedSlots;

        long int incoming_packets;
        long int non_blocked_packets;
        long int blocked_packets;*/
        
        double observed_slots;
        double empty_slots;
                                                                         
        
        double collisions;
        double total_transmissions;
        double successful_transmissions;
        double droppedPackets; //due to retransmissions
        double packetDisposal;
        double driftedSlots;

        double incoming_packets;
        double non_blocked_packets;
        double blocked_packets;

        double txDelay;
        double throughput;
        double staDelay; //overall station's delay
        double blockingProbability;
        
        float slotDrift;
        float driftProbability; //system slot drift probability
        
        //temporal statistics
        int finalBackoffStage;
        int qEmpty;
        int qSize;
        //
        
        //Protocol picking
        int cut;
        int DCF;
        
        //aggregation settings
        int maxAggregation;

    private:
        int backoff_counter;
        int backoff_stage;
        int backlogged;

        int txAttempt;	

        Packet packet;
        FIFO <Packet> MAC_queue;

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
	if(node_id < cut)
	{
		//cout << node_id << ": I am using DCF";
		DCF = 1;
		system_stickiness = 0;
		station_stickiness = 0;
		hysteresis = 0;
		//Set to 1 when trying maximum aggregation in mixed scenario. 0 Otherwise
		if(maxAggregation > 0)
		{
			fairShare = 1;
			//cout << " with maximum aggregation" << endl;
		}else
		{
			fairShare = 0;
			//cout << " without aggregation" << endl;
		}
	}else
	{
		//cout << node_id << ": I am not using DCF" << endl;
		DCF = 0;
	}
	
    backoff_counter = (int)Random(pow(2,backoff_stage)*CWMIN);
    backoff_stage = 0;
    packet.source = node_id;
    packet.L = 1024;
    packet.send_time = SimTime();
    
    observed_slots = 0;
    empty_slots = 0;
    
    txAttempt = 0;
    collisions = 0;
    successful_transmissions = 0;
    droppedPackets = 0;
    packetDisposal = 0;
    total_transmissions = 0;

    incoming_packets = 0;
    non_blocked_packets = 0;
    blocked_packets = 0;
    blockingProbability = 0;

    txDelay = 0;
    
    throughput = 0;
    staDelay = 0;
    
    slotDrift = 0;
    driftedSlots = 0;
    
    //statistics
    finalBackoffStage = 0;
    qEmpty = 0;
    //
};

void STA :: Stop()
{
    
    throughput = packet.L*8*(float)successful_transmissions / SimTime();
    qSize = MAC_queue.QueueSize();
    
    blockingProbability = (float)blocked_packets / (float)incoming_packets;
    
    if(successful_transmissions > 0)
    {
    	staDelay = (float)txDelay / (float)successful_transmissions;
    }else
    {
    	staDelay = 0;
    }
    
    //temporal statistics
    finalBackoffStage = backoff_stage;
    //
    
    cout << endl;
    cout << "--- Station " << node_id << " stats ---" << endl;
    cout << "Total Transmissions:" << " " <<  total_transmissions << endl;
    if(total_transmissions > 0)
    {
    	cout << "Collisions:" << " " << collisions << endl;
    	cout << "Packets successfully sent:" << " " << successful_transmissions << endl;        
    	cout << "*** DETAILED ***" << endl;
    	cout << "TAU = " << (float)total_transmissions / (float)observed_slots << " |" << " p = " << (float)collisions / (float)total_transmissions << endl;
    	cout << "Throughput of this station = " << throughput << "bps" << endl;
    	cout << "Blocking Probability = " << blockingProbability << endl;
    	cout << "Average Delay (queueing + service) = " << staDelay << endl;
    	cout << endl;
    }
    
    cout <<"-----Debug-----"<<endl;
    if(DCF == 1)
	{
		//This node will pick DCF as its contention protocol
		cout << "I am using DCF" << endl;	
	}else
	{
		cout << "I am using something different" << endl;
	}
	cout << "Final backoff stage: " << finalBackoffStage << endl;
    cout << "System stickiness: " << system_stickiness << endl;
    cout << "Station stickiness: " << station_stickiness << endl;
    cout << "Hysteresis: " << hysteresis << endl;
    cout << "Fair Share: " << fairShare << endl;
    if(qEmpty > 1)
    {
    	cout << "The queue emptied " << qEmpty <<" times" << endl;
    }else
    {
    	cout << "Queue was always full" << endl;
    }
    
};

void STA :: in_slot(SLOT_notification &slot)
{
    observed_slots++;
    //stations that are backlogged will decrement backoff, transmit,
    //and check the result of the last transmission

    if (backlogged == 1)
    {
        if (slot.status == 0)
        {
            backoff_counter--;
            empty_slots++;
        }
        if (slot.status == 1)
        {             
            if (backoff_counter == 0) // I have transmitted
            {
                //Sent as many packets as was set in the past packet's structure
                if(fairShare > 0)
                {
                	if(maxAggregation > 0)
                	{
                		packetDisposal = std::min((int)pow(2,MAXSTAGE),MAC_queue.QueueSize());
                		successful_transmissions += packetDisposal;
                		//Deleting as many packets as maxAggregation
                		for(int i = 0; i < packetDisposal; i++)
                		{
                			txDelay += SimTime() - packet.queuing_time;
                			MAC_queue.DelFirstPacket();
                			if(i < packetDisposal-1) packet = MAC_queue.GetFirstPacket();
                    	}
                	}else
                	{
                		packetDisposal = std::min((int)pow(2,backoff_stage),MAC_queue.QueueSize());
                		successful_transmissions += packetDisposal;
                		//if(node_id==10)cout << "Disposal: " << packetDisposal << ", Q: " << MAC_queue.QueueSize() << endl;
                		//Deleting as many packets as the aggregation field in the sent packet structure
                		for(int i = 0; i < packetDisposal; i++)
                		{
                			txDelay += SimTime() - packet.queuing_time;
                			MAC_queue.DelFirstPacket();
                			if(i < packetDisposal-1) packet = MAC_queue.GetFirstPacket();
                    	}
                	}
                	packetDisposal = 0;
                }else
                {
                    successful_transmissions++;
                    txDelay += SimTime() - packet.queuing_time;
                    MAC_queue.DelFirstPacket();
                }
                
                txAttempt = 0;
                
                if(hysteresis == 0) backoff_stage = 0;
                
                //After successful tx, the sta_st goes back to sys_st
                station_stickiness = system_stickiness;
               
                if (MAC_queue.QueueSize() == 0)
                {
                    backlogged = 0;
                    backoff_stage = 0;
                    qEmpty++;
                }else
                {
                    packet = MAC_queue.GetFirstPacket(); //any previously packet in the variable is replaced by a new one
                    packet.send_time = SimTime();
                    backoff_counter = backoff(backoff_stage, station_stickiness, driftProbability);
                }
            }else
            {
                //Other stations transmitted
                //Decrement backoff_counter
                backoff_counter--;           
            }
            
        }
        
        /*--------Collisions----------*/
        
        if (slot.status > 1)
        {   
            if (backoff_counter == 0) // I have transmitted
            {
                txAttempt++;
                collisions++;
                //One collision, one less counter stickiness
                if(system_stickiness > 0){ 
                    station_stickiness = std::max(0, station_stickiness-1);
                    if(station_stickiness == 0)
                    {
                        backoff_stage = std::min(backoff_stage+1,MAXSTAGE);
                        backoff_counter = backoff(backoff_stage, station_stickiness, driftProbability);
                    }else //still sticky
                    {                       
                        //Weird scenario at the moment. Just for a system_stickiness > 1
                        backoff_counter = backoff(backoff_stage, station_stickiness, driftProbability);
                    }
                }else
                {
                    backoff_stage = std::min(backoff_stage+1,MAXSTAGE);
                    backoff_counter = backoff(backoff_stage, station_stickiness, driftProbability);
                }
                    
                                                                  
                if (txAttempt > MAX_RET)
                {
                    txAttempt = 0;
                    //after dropping a frame in DCF, the backoff_stage is reset
                    if(hysteresis == 0) backoff_stage = 0;
                    
                    //Removing as many packets as were supposed to be sent
                    if(fairShare > 0){
                    	if(maxAggregation > 0)
                    	{
                    		packetDisposal = std::min((int)pow(2,MAXSTAGE),MAC_queue.QueueSize());
                    		droppedPackets+=packetDisposal;
                    		for(int i = 0; i < packetDisposal; i++)
                    		{
                    			MAC_queue.DelFirstPacket();
                    		}
                    	}else
                    	{
                    		packetDisposal = std::min((int)pow(2,backoff_stage),MAC_queue.QueueSize());
                    		droppedPackets+=packetDisposal;
                    		for(int i = 0; i < packetDisposal; i++)
                    		{
                    			MAC_queue.DelFirstPacket();
                    		}
                    	}
                    	packetDisposal = 0;
                    }else
                    {
                    	droppedPackets++;
                    	MAC_queue.DelFirstPacket();
                 	}
                 	
                 	if(MAC_queue.QueueSize() > 0)
                 	{
                 		packet = MAC_queue.GetFirstPacket();
                 		packet.send_time = SimTime();
                 	}else
                 	{
                 		backlogged = 0;
                 	}
                }
            }
            else
            {
                //Other stations collided
                backoff_counter--;
            }
        }

    }
    //stations that are not backlogged will wait for a packet
    if (backlogged == 0)
    {
        if (MAC_queue.QueueSize() > 0)
        {
            backlogged = 1;
            packet = MAC_queue.GetFirstPacket();
            packet.send_time = SimTime();
        }
        
    }
    
    //transmit if backoff counter reaches zero and the station has something in the queue
    if ((backoff_counter == 0) && (backlogged == 1))
    {
        total_transmissions++;
        if(fairShare > 0)
        {
        	if(maxAggregation > 0)
        	{
        		packet.aggregation = std::min((int)pow(2,MAXSTAGE),MAC_queue.QueueSize());
        	}else
        	{
        		packet.aggregation = std::min((int)pow(2,backoff_stage),MAC_queue.QueueSize());
        	}
        }else
        {
            packet.aggregation = 1;
        }
        
        out_packet(packet);
    }
    
};

void STA :: in_packet(Packet &packet)
{
    incoming_packets++;

    if (MAC_queue.QueueSize() < K)
    {
        non_blocked_packets++;
        packet.queuing_time = SimTime();
        MAC_queue.PutPacket(packet);
    }else
    {
        blocked_packets++;
    }
}

