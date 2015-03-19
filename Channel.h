/*
	Channel Component
*/

#define DATARATE 11E6 // Data Transmission Rate
#define PHYRATE 1E6

/*
#define SLOT 20E-6 // Empty Slot
#define DIFS 50E-6
#define SIFS 10E-6
#define L_ack 112*/

//Complying with 802.11n at 2.4 GHz
#define SLOT 9e-06 
#define DIFS 28e-06
#define SIFS 10e-06
#define LDBPS 256
#define TSYM 4e-06
#define ECA_AIFS 28e-06
			
#include "Aux.h"

component Channel : public TypeII
{

	public:
		void Setup();
		void Start();
		void Stop();
			
	public:
		int Nodes;
		float error;

		// Connections
		outport [] void out_slot(SLOT_notification &slot);	
		inport void in_packet(Packet &packet);

		// Timers
		Timer <trigger_t> slot_time; // Duration of current slot
		Timer <trigger_t> rx_time; // Time to receive all packets transmitted in current slot
		// Timer <trigger_t> cpSampler; // Sampler of the collision probability		
		
		inport inline void NewSlot(trigger_t& t1);
		inport inline void EndReceptionTime(trigger_t& t2);
		// inport inline void Sampler(trigger_t& t3);

		Channel () { 
			connect slot_time.to_component,NewSlot; 
			connect rx_time.to_component,EndReceptionTime;
		    // connect cpSampler.to_component,Sampler; 
		}

	private:
		double number_of_transmissions_in_current_slot;
		double succ_tx_duration, collision_duration; // Depends on the packet(s) size(s)
		double empty_slot_duration;
		double TBack;
		double L_max;
		int MAC_H, PCLP_PREAMBLE, PCLP_HEADER;
		int aggregation;
		float errorProbability;
		
		//gathering statistics about the collision's evolution in time
     	ofstream slotsInTime;

	public: // Statistics
		double collision_slots, empty_slots, succesful_slots, total_slots;
		double totalBitsSent;
		double recentCollisions; //Collisions during the last 1000 slots
		int test;
};

void Channel :: Setup()
{

};

void Channel :: Start()
{
	number_of_transmissions_in_current_slot = 0;
	succ_tx_duration = 10E-3;
	collision_duration = 10E-3;
	empty_slot_duration = 9e-06;

	collision_slots = 0;
	empty_slots = 0;
	succesful_slots = 0;
	total_slots = 0;
	test = 0;
	recentCollisions = 0;

	L_max = 0;
	
	MAC_H = 272;
	PCLP_PREAMBLE = 144; 
	PCLP_HEADER = 48;
	
	TBack = 32e-06 + ceil((16 + 256 + 6)/LDBPS) * TSYM;
	totalBitsSent = 0;

	aggregation = 0;
	errorProbability = 0;

	slot_time.Set(SimTime()); // Let's go!	
	
	slotsInTime.open("Results/slotsInTime.txt", ios::app);

};

void Channel :: Stop()
{
	printf("\n\n");
	printf("---- Channel ----\n");
	printf("Slot Status Probabilities (channel point of view): Empty = %e, Succesful = %e, Collision = %e \n",empty_slots/total_slots,succesful_slots/total_slots,collision_slots/total_slots);
	printf("Total packets sent to the Channel: %d\n", (int)succesful_slots);
	printf("\n\n");


	
	slotsInTime.close();
};

void Channel :: NewSlot(trigger_t &)
{
	//printf("%f ***** NewSlot ****\n",SimTime());

	SLOT_notification slot;

	slot.status = number_of_transmissions_in_current_slot;

	number_of_transmissions_in_current_slot = 0;
	L_max = 0;

	for(int n = 0; n < Nodes; n++) out_slot[n](slot); // We send the SLOT notification to all connected nodes

	rx_time.Set(SimTime());	// To guarantee that the system works correctly. :)
}

void Channel :: EndReceptionTime(trigger_t &)
{
    //Slots are different than frames. We can transmit multiple frames in one long slot through aggregation


	if(number_of_transmissions_in_current_slot==0) 
	{
		slot_time.Set(SimTime()+SLOT);
		empty_slots++;
	}
	if(number_of_transmissions_in_current_slot == 1)
	{
		slot_time.Set(SimTime()+succ_tx_duration);
		succesful_slots++;
		totalBitsSent += aggregation*(L_max*8);
	}
	if(number_of_transmissions_in_current_slot > 1)
	{
		slot_time.Set(SimTime()+collision_duration);
		collision_slots++;	
		recentCollisions++;
	}
		
	total_slots++; //Just to control that total = empty + successful + collisions
	
	
	//Used to plot slots vs. collision probability
	
    if(int(total_slots) % 1000 == 0) //just printing in thousands increments
	{
	        slotsInTime << Nodes << " " << SimTime() << " " <<  total_slots << " " << collision_slots/total_slots << " " 
	        	<< succesful_slots/total_slots << " " << empty_slots/total_slots << " " << recentCollisions << endl;

        	recentCollisions = 0;
	}
	
}


void Channel :: in_packet(Packet &packet)
{
	//cout << "Received Packet from node: " << packet.source << endl;

	if(packet.L > L_max) L_max = packet.L;
	
	aggregation = packet.aggregation;
	
	errorProbability = rand() % (int) 100;
	
	if( (errorProbability > 0) && (errorProbability <= (int)(error*100)) )
	{
	    //If the channel error probability is contained inside the system error margin,
	    //then something wrong is going to happen with the transmissions in this slot
	    number_of_transmissions_in_current_slot+=2;
	}else
	{
	    number_of_transmissions_in_current_slot++;
	}

	switch(packet.fairShare)
	{
		// Uncomment for QoS testings with AIFS
		// case 1:
		// 	succ_tx_duration = ECA_AIFS + 32e-06 + ceil((16 + aggregation*(32+(L_max*8)+288) + 6)/LDBPS)*TSYM + SIFS + TBack + DIFS + empty_slot_duration;
		// 	break;
			
		default:
			succ_tx_duration = (SIFS + 32e-06 + ceil((16 + aggregation*(32+(L_max*8)+288) + 6)/LDBPS)*TSYM + SIFS + TBack + DIFS + empty_slot_duration);
	}

	
	collision_duration = succ_tx_duration;
	
	/*succ_tx_duration = ((PCLP_PREAMBLE + PCLP_HEADER)/PHYRATE) + ((aggregation*L_max*8 + MAC_H)/DATARATE) + SIFS + ((PCLP_PREAMBLE + PCLP_HEADER)/PHYRATE) + (L_ack/PHYRATE) + DIFS;
	collision_duration = ((PCLP_PREAMBLE + PCLP_HEADER)/PHYRATE) + ((aggregation*L_max*8 + MAC_H)/DATARATE) + SIFS + DIFS + ((144 + 48 + 112)/PHYRATE);*/
	
	
}

