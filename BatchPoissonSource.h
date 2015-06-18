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
		Timer <trigger_t> inter_packet_timerBK;
		Timer <trigger_t> inter_packet_timerBE;
		Timer <trigger_t> inter_packet_timerVI;
		Timer <trigger_t> inter_packet_timerVO;
		
		inport inline void new_packetBK(trigger_t& tBK);
		inport inline void new_packetBE(trigger_t& tBE);
		inport inline void new_packetVI	(trigger_t& tVI);
		inport inline void new_packetVO(trigger_t& tVO);

		BatchPoissonSource () { 
			connect inter_packet_timerBK.to_component,new_packetBK;
			connect inter_packet_timerBE.to_component,new_packetBE;
			connect inter_packet_timerVI.to_component,new_packetVI;
			connect inter_packet_timerVO.to_component,new_packetVO; }

	public:
		int L;
		long int seqBK;
		long int seqBE;
		long int seqVI;
		long int seqVO;
		int MaxBatch;	
		double packet_rateBK;
		double packet_rateBE;
		double packet_rateVI;
		double packet_rateVO;
	
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
	if(packet_rateBK > 0) inter_packet_timerBK.Set(Exponential(1/packet_rateBK));
	seqBK = 0;
	
	if(packet_rateBE > 0) inter_packet_timerBE.Set(Exponential(1/packet_rateBE));
	seqBE = 0;
		
	if(packet_rateVI > 0) inter_packet_timerVI.Set(Exponential(1/packet_rateVI));
	seqVI = 0;
	
	if(packet_rateVO > 0) inter_packet_timerVO.Set(Exponential(1/packet_rateVO));
	seqVO = 0;	
};
	
void BatchPoissonSource :: Stop()
{

};

void BatchPoissonSource :: new_packetBK(trigger_t &)
{
	Packet packetBK;

	packetBK.L = L;
	packetBK.accessCategory = 1;
			
	int RB = (int) Random(MaxBatch)+1;

	for(int p=0; p < RB; p++)
	{
		packetBK.seq = seqBK;
		packetBK.queuing_time = SimTime();

		out(packetBK);

		seqBK++;

	}
	
	inter_packet_timerBK.Set(SimTime()+Exponential(RB/packet_rateBK));	

};

void BatchPoissonSource :: new_packetBE(trigger_t &)
{
	Packet packetBE;
	packetBE.accessCategory = 0;

	packetBE.L = L;
			
	int RB = (int) Random(MaxBatch)+1;

	for(int p=0; p < RB; p++)
	{
		packetBE.seq = seqBE;
		packetBE.queuing_time = SimTime();

		out(packetBE);

		seqBE++;

	}
	
	inter_packet_timerBE.Set(SimTime()+Exponential(RB/packet_rateBE));

};

void BatchPoissonSource :: new_packetVI(trigger_t &)
{
	Packet packetVI;
	packetVI.accessCategory = 2;

	packetVI.L = L;
			
	int RB = (int) Random(MaxBatch)+1;

	for(int p=0; p < RB; p++)
	{
		packetVI.seq = seqVI;
		packetVI.queuing_time = SimTime();

		out(packetVI);

		seqVI++;

	}
	
	inter_packet_timerVI.Set(SimTime()+Exponential(RB/packet_rateVI));	

};

void BatchPoissonSource :: new_packetVO(trigger_t &)
{
	Packet packetVO;
	packetVO.accessCategory = 3;

	packetVO.L = L;
			
	int RB = (int) Random(MaxBatch)+1;

	for(int p=0; p < RB; p++)
	{
		packetVO.seq = seqVO;
		packetVO.queuing_time = SimTime();

		out(packetVO);

		seqVO++;

	}
	
	inter_packet_timerVO.Set(SimTime()+Exponential(RB/packet_rateVO));	

};

