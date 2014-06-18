/*
	Poisson source. 
*/
			
#include "Aux.h"

#define MAXSEQ 1024

component BatchPoissonSource : public TypeII
{
	public:
		// Connections
		outport void out(Packet &packet);	

		// Timer
		Timer <trigger_t> inter_packet_timer;
		inport inline void new_packet(trigger_t& t);

		BatchPoissonSource () { 
			connect inter_packet_timer.to_component,new_packet; }

	public:
		int L;
		int seq;
		double bandwidth; // Source Bandwidth
		double packet_rate;
		int MaxBatch;
		int aggregation;
	
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
	packet_rate=bandwidth/(L*8);
	inter_packet_timer.Set(Exponential(1/packet_rate));
	seq=0;
};
	
void BatchPoissonSource :: Stop()
{

};

void BatchPoissonSource :: new_packet(trigger_t &)
{
	Packet packet;

	packet.L = L;
			
	int RB = (int) Random(MaxBatch)+1;

	for(int p=0; p < RB; p++)
	{
		packet.seq = seq;
		packet.send_time = SimTime();

		out(packet);

		seq++;
		if(seq == MAXSEQ) seq = 0;

	}
	
	inter_packet_timer.Set(SimTime()+Exponential(RB/packet_rate));	

};



