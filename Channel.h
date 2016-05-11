/*
	Channel Component
*/
#define AC 4
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
#define DIFS11 50e-06
#define SIFS 10e-06
#define LDBPS 256
#define TSYM 4e-06
#define TSYM11ax 16e-06
#define ECA_AIFS_TIME 28e-06
#define SIG_EXT 6e-06

//New DIFS and SIFS for HEW protocols
#define SIFSHEW 16e-06
#define DIFSHEW 34e-06
			
#include "Aux.h"
#include "includes/subCarriers.hh"
#include "includes/subCarriers11ax.hh"

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
		Timer <trigger_t> triggerChange; // Sampler of the collision probability		
		
		inport inline void NewSlot(trigger_t& t1);
		inport inline void EndReceptionTime(trigger_t& t2);
		inport inline void changeConditions(trigger_t& t3);

		Channel () { 
			connect slot_time.to_component,NewSlot; 
			connect rx_time.to_component,EndReceptionTime;
		    connect triggerChange.to_component,changeConditions; 
		}

	private:
		double number_of_transmissions_in_current_slot;
		int affected;	//number of packets affected by a channel error
		std::vector<int> affectedFrames;
		double succ_tx_duration, collision_duration; // Depends on the packet(s) size(s)
		double empty_slot_duration;
		double TBack;
		double L_max;
		int MAC_H, PLCP_PREAMBLE, PLCP_HEADER;
		int aggregation;
		float errorProbability;
		int rate;	//	are we using the 48mbps metrics for tx duration?
		int channelWidth; // bandwidth
		bool QoS; // RTS/CTS
		std::vector<double> biggestTxDuration, biggestFrameSize;
		
		//gathering statistics about the collision's evolution in time
     	ofstream slotsInTime;

	public: // Statistics
		double collision_slots, empty_slots, succesful_slots, error_slots, total_slots;
		double totalBitsSent;
		double recentCollisions; //Collisions during the last 1000 slots
		double pastRecentCollisions;
		double recentErrors;
		int errorPeriod;
		bool channelModel;	//0 = perfect, 1 = bad
		signed long long slotNum;
};

void Channel :: Setup()
{

};

void Channel :: Start()
{
	number_of_transmissions_in_current_slot = 0;
	affected = 0;
	succ_tx_duration = 0.0;
	collision_duration = 0.0;
	empty_slot_duration = 9e-06;

	collision_slots = 0;
	empty_slots = 0;
	succesful_slots = 0;
	total_slots = 0;
	error_slots = 0;
	recentCollisions = 0;
	pastRecentCollisions = 0;
	recentErrors = 0;
	slotNum = 0;
	errorPeriod = 0;
	channelModel = true;
	triggerChange.Set(SimTime());

	L_max = 0;
	
	MAC_H = 272;
	PLCP_PREAMBLE = 144; 
	PLCP_HEADER = 48;
	
	TBack = 32e-06 + ceil((16 + 256 + 6)/LDBPS) * TSYM;
	totalBitsSent = 0;

	aggregation = 1;
	errorProbability = 0;
	affectedFrames.assign (aggregation, 0);
	biggestFrameSize.assign (AC, 0.0);
	biggestTxDuration.assign (AC, 0.0);

	rate = 1000; // mark 100 for 802.11ac and 1000 for 802.11ax
	QoS = true;
	channelWidth = 20; // MHz

	slot_time.Set(SimTime()); // Let's go!	
	
	slotsInTime.open("Results/slotsInTime.txt", ios::app);

};

void Channel :: Stop()
{
	printf("\n\n");
	printf("---- Channel ----\n");
	printf("Slot Status Probabilities (channel point of view): Empty = %e, Succesful = %e, Collision = %e \n",empty_slots/total_slots,succesful_slots/total_slots,collision_slots/total_slots);
	printf("Total packets sent to the Channel: %d\n", (int)succesful_slots);
	printf("Longest transmission durations and biggest frame sizes per AC:\n");
	for (int i = 0; i < AC; i++)
	{
		cout << "\t-AC-" << i << ": " << biggestTxDuration.at(i) << "s. Size: " << biggestFrameSize.at(i) << "B." << endl;
	}
	printf("\n\n");


	
	slotsInTime.close();
};

void Channel :: changeConditions(trigger_t &)
{
	if(error > 0 && errorPeriod > 0)
	{
		channelModel = !channelModel;
		//*/DEBUG
		// cout << "(" << SimTime() << ") Chaging from " << !channelModel << " to " << channelModel << endl; 
		triggerChange.Set(SimTime() + errorPeriod);
	}

}

void Channel :: NewSlot(trigger_t &)
{
	//printf("%f ***** NewSlot ****\n",SimTime());

	SLOT_notification slot;
	slotNum++;

	slot.status = number_of_transmissions_in_current_slot;
	slot.num = slotNum;
	slot.error = affected;
	slot.affectedFrames.assign(affectedFrames.size (),0); 
	slot.affectedFrames = affectedFrames;
	
	number_of_transmissions_in_current_slot = 0;
	affected = 0;
	affectedFrames.assign (1,0);
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
		if(affected > 0)
		{
			error_slots++;
			recentErrors++;
			totalBitsSent += (aggregation - affected)*(L_max*8);

		}else
		{
			totalBitsSent += aggregation*(L_max*8);
			succesful_slots++;
		}
	}
	if(number_of_transmissions_in_current_slot > 1)
	{
		collision_slots++;	
		recentCollisions++;
		slot_time.Set(SimTime()+collision_duration);
	}
		
	total_slots++; //Just to control that total = empty + successful + collisions
	
	
	//Used to plot slots vs. collision probability
	int increment = 100;
    if(int(total_slots) % increment == 0) //just printing in increments
	{
	        slotsInTime << Nodes << " " << SimTime() << " " <<  total_slots << " " << collision_slots/total_slots << " " 
	        	<< succesful_slots/total_slots << " " << empty_slots/total_slots << " " << recentCollisions << " " 
	        	<< error_slots/total_slots << " " << recentErrors << endl;

	        if(pastRecentCollisions == 0)
	        {
	        	pastRecentCollisions = collision_slots;	
	        }else
	        {
	        	if(pastRecentCollisions == collision_slots)
	        	{
	        		// cout << SimTime() << endl;
	        	}else
	        	{
	        		pastRecentCollisions = collision_slots;
	        	}
	        }

        	recentCollisions = 0;
        	recentErrors = 0;
	}
	
}


void Channel :: in_packet(Packet &packet)
{
	if(packet.L > L_max) L_max = packet.L;
	
	aggregation = packet.aggregation;

	int model = channelModel;
	int ac = packet.accessCategory;

	switch(model)
	{

		case false:
			number_of_transmissions_in_current_slot++;
			break;

		case true:
			affectedFrames.clear();
			affectedFrames.assign (aggregation, 0);
			for(int i = 0; i < aggregation; i++)
			{
			    //If the channel error probability is contained inside the system error margin,
			    //then something wrong is going to happen with the transmissions in this slot
				//In this case we decide if certain packet is affected or not by the errors
				errorProbability = rand() % (int) 100;
				if( (errorProbability > 0) && (errorProbability <= (int)(error*100)) )
				{
					affectedFrames.at(i) = 1;
					affected++;
				}else
				{
					packet.allSeq.at(i) = 0;
				}
			}
			if(affected > 0)
			{
				if(affected == aggregation)
				{
					number_of_transmissions_in_current_slot++; //Add an additional tx to mimic a collision
				}	
			}
		    number_of_transmissions_in_current_slot++;
			break;
		default:
			number_of_transmissions_in_current_slot++;
			break;
	}

	double ACK;
	double frame;
	//For 802.11ac and ax:
	double ofdmBits;
	double basicOfdmRate;
	int Ysb; // number of subcarriers
	double Ym = 6; // bits / symbol
	double Yc = 3.0/4.0; // Coding rate
	double Nu = 1; //number of users
	double T_RTS, T_CTS, T_BAR, T_BA;
	double RTS = 160;
	double CTS = 128;
	double MH = 240; // MAC header in bits
	double SF = 16; // service field?
	double TB = 18;
	double MD = 32; // MAC delimeter
	double BA = 30 * 8; //Compressed version
	double BAR = 24 * 8;

	int M = 4; //number of antennas
	double phy;
	int variableAggregation = packet.QoS;
	int coefficient = 1;

	switch(rate)
	{
		case 48:
			//JUST FOR SINGLE FRAME TRANSMISSIONS.
			//IT CURRENTLY DOES NOT SUPPORT AGGREGATION

			//Calculating the duration of a transmission in 48Mbps according to durations-wifi-ofdm.xlsx
			// (bits for ack are: service, MAC, FCS, tail)
			// ACK = PLCP + ceil((bits)/NDBPS)* SymbolTime + pause
			ACK = 20.0 + ceil(134.0/192.0) * 4.0 + 6.0;

			// (bits are: service, MAC Header, LLC Header, IP Header, UDP Header, PAYLOAD, FCS, Tail).
			// T = PLCP + ceil((bits)/NDBPS)* SymbolTime + pause + SIFS + ACK + DIFS + CWaverage*SLOT;
			frame = 20.0 + ceil((L_max*8.0 + 534.0)/192.0) * 4.0 + 6.0;

			//Last 9 is an empty slot and 7.5 is CWmin/2
			succ_tx_duration = frame * 1e-06 + SIFS + ACK * 1e-06 + DIFS + 9.0 * 7.5 * 1e-06;
			collision_duration = succ_tx_duration;

			// cout << succ_tx_duration << endl;
			break;
		case 11:
			//JUST FOR SINGLE FRAME TRANSMISSIONS.
			//IT CURRENTLY DOES NOT SUPPORT AGGREGATION
			//Calculating the duration of a transmission in 11Mbps according to 
			// 80211_TransmissionTime.xls and durations-wifi-ofdm.xlsx
			// (bits ack: Service, MAC, FCS) = 14 bytes.
			// ACK = PLCP + PLCPpreable + ceil((bits)/rate)
			ACK = 24 + 72 + ceil((14*8.0)/11.0);
			// (bits are: MAC Header, LLC Header, IP header, UDP header, PAYLOAD, FCS)
			// frame = PLCP + PLCPpreable + ceil((bits)/rate)
			frame = 24 + 72 + ceil(((24 + 8 + 20 + 8 + L_max + 4)*8.0) / 11.0);
			// T = frame + SIFS + ACK + DIFS + CWaverage*SLOT;
			//Last 9 is an empty slot and 7.5 is CWmin/2
			succ_tx_duration = frame * 1e-06 + SIFS + ACK * 1e-06 + DIFS11 + 9.0 * 7.5 * 1e-06;
			collision_duration = succ_tx_duration;
			break;
		case 65:
			// succ_tx_duration = (SIFS + 32e-06 + ceil((16 + aggregation*(32+(L_max*8)+288) + 6)/LDBPS)*TSYM + SIFS + TBack + DIFS + empty_slot_duration);
			//This is the TON version

			//Here, variable aggregation is wrong!
			if (variableAggregation > 0)
			{
				coefficient = variableAggregation;
			}else
			{
				coefficient = aggregation;
			}
			succ_tx_duration = (SIFS + 32e-06 + ceil((16 + coefficient*(32+(L_max*8)+288) + 6)/LDBPS)*TSYM + SIFS + TBack + DIFS + empty_slot_duration);
			collision_duration = succ_tx_duration;
			break;
		case 100:
			// 802.11ac according to Boris's calculations
			//succ_tx_duration = T_RTS + SIFS + T_CTS + SIFS + frame + SIFS + T_BAR + SIFS + T_BA + DIFS + SLOT
			Ysb = subCarriers (channelWidth);
			ofdmBits = Ysb * Ym * Yc * Nu;
			basicOfdmRate = subCarriers (20) * 1 * 1/2;
			phy = 36e-6 + (M * TSYM);
			T_RTS = phy + ceil ((SF + RTS + TB) / basicOfdmRate) * TSYM;
			T_CTS = phy + ceil ((SF + CTS + TB) / basicOfdmRate) * TSYM;
			T_BAR = phy + ceil ((SF + BAR + TB) / basicOfdmRate) * TSYM;
			T_BA = phy + ceil ((SF + BA + TB) / basicOfdmRate) * TSYM;

			if (aggregation > 1)
			{
				frame = phy + ceil ((SF + aggregation * (MD + MH) + (L_max * 8.0) + TB) / ofdmBits) * TSYM;
			}else
			{
				frame = phy + (ceil ((SF + MH + (L_max * 8.0) + TB) / ofdmBits)) * TSYM;

			}
			if (!QoS)
			{
				T_RTS = 0.0;
				T_CTS = 0.0;
			}

			succ_tx_duration = T_RTS + SIFSHEW + T_CTS + SIFSHEW + frame + SIFSHEW + T_BAR + SIFSHEW + T_BA + DIFSHEW + SLOT;
			collision_duration =  T_RTS + SIFSHEW + T_CTS + DIFSHEW + SLOT;
			break;
		case 1000:
			// 802.11ax according to Boris's calculations
			// succ_tx_duration = T_RTS + SIFS + T_CTS + SIFS + T_DATA + SIFS + T_BAR + SIFS + T_BA + DIFS + Te
			Ysb = subCarriers11ax (channelWidth);
			ofdmBits = Ysb * Ym * Yc * Nu;
			basicOfdmRate = subCarriers11ax (20) * 1 * 1/2;
			phy = 36e-6 + (M * TSYM11ax);
			T_RTS = phy + ceil ((SF + RTS + TB) / basicOfdmRate) * TSYM11ax;
			T_CTS = phy + ceil ((SF + CTS + TB) / basicOfdmRate) * TSYM11ax;
			T_BAR = phy + ceil ((SF + BAR + TB) / basicOfdmRate) * TSYM11ax;
			T_BA = phy + ceil ((SF + BA + TB) / basicOfdmRate) * TSYM11ax;
			if (aggregation > 1)
			{
				frame = phy + ceil ((SF + aggregation * (MD + MH) + (L_max*8) + TB) / ofdmBits) * TSYM11ax;
			}else
			{
				frame = phy + ceil ((SF + MH + (L_max * 8) + TB) / ofdmBits) * TSYM11ax;
			}
			if (!QoS)
			{
				T_RTS = 0.0;
				T_CTS = 0.0;
				succ_tx_duration = T_RTS + SIFSHEW + T_CTS + SIFSHEW + frame + SIFSHEW + T_BAR + SIFSHEW + T_BA + DIFSHEW + SLOT;
				collision_duration = succ_tx_duration;
			}else
			{
				succ_tx_duration = T_RTS + SIFSHEW + T_CTS + SIFSHEW + frame + SIFSHEW + T_BAR + SIFSHEW + T_BA + DIFSHEW + SLOT;
				collision_duration =  T_RTS + SIFSHEW + T_CTS + DIFSHEW + SLOT;
			}
			break;
		default:
			break;
	}
	if (number_of_transmissions_in_current_slot == 1)
	{
		if (biggestTxDuration.at(ac) < succ_tx_duration)
			biggestTxDuration.at(ac) = succ_tx_duration;
		if (packet.L > biggestFrameSize.at(ac))
			biggestFrameSize.at(ac) = packet.L;
	}

	// cout << SimTime() << "- Channel, Ac-" << packet.accessCategory << ": first Seq: " << packet.firstMPDUSeq << ": last seq: " << packet.lastMPDUSeq << ": Aggregation: " << aggregation << ": Load: " << L_max 
	// << ", duration: " << succ_tx_duration << endl;
}

