/*
	Poisson source. 
*/
			
#include "Aux.h"

#define MAXSEQ 1024
#define AC 4

component BatchPoissonSource : public TypeII
{
	public:
		// Connections
		outport void out(Packet &packet);	

		// Timer
		Timer <trigger_t> inter_packet_timer;
		
		inport inline void new_packet(trigger_t& t);

		BatchPoissonSource () { 
			connect inter_packet_timer.to_component,new_packet;
		}

	public:
		int L;
		long int seq;
		int MaxBatch;	
		double packet_rate;
		int categories;
		int packetGeneration;
		double packetsGenerated;
		std::array<double,AC> packetsInAC;

		int BEShare;
		int BKShare;
		int VIShare;
		int VOShare;
	
	public:
		void Setup();
		void Start();
		void Stop();
			
};

void BatchPoissonSource :: Setup()
{

};

void BatchPoissonSource :: Start()
{
	if(packet_rate > 0) inter_packet_timer.Set(Exponential(1/packet_rate));
	seq = 0;
};
	
void BatchPoissonSource :: Stop()
{

};

void BatchPoissonSource :: new_packet(trigger_t &)
{
	packetGeneration = rand() % (int) (100);
	Packet packet;

	switch(categories)
	{
		case 1:
			packet.accessCategory = 0;
			break;
		case 2:
			if( (packetGeneration >= VIShare) && (packetGeneration < BEShare) )
			{
				packet.accessCategory = 1;
			}else
			{
				packet.accessCategory = 0;
			}
			break;
		case 3:
			if( (packetGeneration >= VOShare) && (packetGeneration < VIShare) )
			{
				packet.accessCategory = 2;
			}else if( (packetGeneration >= VIShare) && (packetGeneration < BEShare) )
			{
				packet.accessCategory = 1;
			}else
			{
				packet.accessCategory = 0;
			}
			break;
		default:
			if(packetGeneration < VOShare)
			{
				packet.accessCategory = 3;
			}else if( (packetGeneration >= VOShare) && (packetGeneration < VIShare) )
			{
				packet.accessCategory = 2;
			}else if( (packetGeneration >= VIShare) && (packetGeneration < BEShare) )
			{
				packet.accessCategory = 1;
			}else
			{
				packet.accessCategory = 0;
			}
			break;
	}

	int RB = (int) Random(MaxBatch)+1;
	
	packet.L = L;
	packet.queuing_time = SimTime();
	out(packet);
	packetsGenerated += 1;
	packetsInAC.at(packet.accessCategory) += 1;

	inter_packet_timer.Set(SimTime()+Exponential(RB/packet_rate));	
};

