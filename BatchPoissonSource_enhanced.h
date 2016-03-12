/*
	Poisson source. 
*/
			
#include "Aux.h"

#define MAXSEQ 1024
#define AC 4
//a 16 frames GOP, with 3 B frames (G16-B3), H.264/AVC
#define GOPSIZE 16
#define Iav 31992
#define Pav 23463
#define Bav 2623
// IBPBPBPBPBPBPBPB
extern "C" const double gop [GOPSIZE] = {Iav,Bav,Pav,Bav,Pav,Bav,Pav,Bav,Pav,Bav,Pav,Bav,Pav,Bav,Pav,Bav};

component BatchPoissonSource : public TypeII
{
	public:
		// Connections
		outport void out(Packet &packet);	

		// Timer
		Timer <trigger_t> inter_packet_timer;
		Timer <trigger_t> on_off_VO;
		Timer <trigger_t> on_off_VI;
		Timer <trigger_t> source_VO;
		Timer <trigger_t> source_VI;
		Timer <trigger_t> source_BE;
		Timer <trigger_t> source_BK;

		inport inline void new_packet(trigger_t& t);
		inport inline void new_VO_packet(trigger_t& t0);
		inport inline void new_VI_packet(trigger_t& t1);
		inport inline void new_BE_packet(trigger_t& t2);
		inport inline void new_BK_packet(trigger_t& t3);
		inport inline void onOffVO(trigger_t& t4);
		inport inline void onOffVI(trigger_t& t5);


		BatchPoissonSource () { 
			connect inter_packet_timer.to_component,new_packet;
			connect on_off_VO.to_component,onOffVO;
			connect on_off_VI.to_component,onOffVI;
			connect source_VO.to_component,new_VO_packet;
			connect source_VI.to_component,new_VI_packet;
			connect source_BE.to_component,new_BE_packet;
			connect source_BK.to_component,new_BK_packet;
		}

	public:
		int L;
		long int seq;
		int MaxBatch;	
		double packet_rate;
		double vo_rate, vi_rate;
		double g711Payload;
		double g711PacketInterval;
		double lambda;
		int categories;
		int packetGeneration;
		double packetsGenerated;
		int alwaysSat;
		bool QoS;
		std::array<double,AC> packetsInAC;

		//H.264/AVC
		int gopPos;

		int BEShare;
		int BKShare;
		int VIShare;
		int VOShare;

		double onPeriodVO, onPeriodVI, offPeriodVO, offPeriodVI;
		double lastSwitchVO, lastSwitchVI;
		bool onVO, onVI;
	
	public:
		void Setup();
		void Start();
		void Stop();
		void registerStatistics(Packet &p);
		double pickFrameFromH264Gop (int &pos);
			
};

void BatchPoissonSource :: registerStatistics (Packet &p)
{
	packetsGenerated += 1;
	packetsInAC.at(p.accessCategory) += 1;
}

double BatchPoissonSource :: pickFrameFromH264Gop (int &pos)
{
	double size = 0.0;
	size = gop[pos];

	pos++;
	if (pos == GOPSIZE) pos = 0;
	return size;
}

void BatchPoissonSource :: Setup()
{

};

void BatchPoissonSource :: Start()
{
	onPeriodVO = 0.3;
	offPeriodVO = 0.5;
	onPeriodVI = 1;
	offPeriodVI = 0.1;
	onVO = true;
	onVI = true;

	//Defined by G.711 packet size and source rate
	g711Payload = 80 * 8;
	g711PacketInterval = 10e-3;
	vo_rate = g711Payload / g711PacketInterval;
	if (QoS)
	{
		gopPos = 0;
		source_BE.Set(Exponential(1/packet_rate));
		source_BK.Set(Exponential(1/packet_rate));
		on_off_VI.Set(SimTime ());
		on_off_VO.Set(SimTime ());
		lastSwitchVO = SimTime ();
		lastSwitchVI = SimTime ();
	}else
	{
		inter_packet_timer.Set(Exponential(1/packet_rate));
	}

	seq = 0;
};
	
void BatchPoissonSource :: Stop()
{

};

void BatchPoissonSource :: onOffVO(trigger_t &)
{
	if (onPeriodVO > 0)
	{
		if (onVO)
		{
			if (SimTime () - lastSwitchVO >= onPeriodVO)
			{
				onVO = false;
				on_off_VO.Set(SimTime() + offPeriodVO);
				lastSwitchVO = SimTime();
			}else
			{
				source_VO.Set(SimTime());
				on_off_VO.Set(SimTime() + onPeriodVO);
			}
		}else
		{
			if (SimTime () - lastSwitchVO >= offPeriodVO)
			{
				onVO = true;
				source_VO.Set(SimTime());
				on_off_VO.Set(SimTime() + onPeriodVO);
			}
		}
	}else
	{
		source_VO.Set(SimTime());
	}
}

void BatchPoissonSource :: new_VO_packet(trigger_t &)
{
	Packet pkt;
	if (onVO)
	{
		pkt.accessCategory = 3;
		pkt.L = g711Payload;
		out (pkt);
		registerStatistics (pkt);
		source_VO.Set(SimTime () + Exponential(1/vo_rate));
	}
}

void BatchPoissonSource :: onOffVI(trigger_t &)
{
	if (onPeriodVI > 0)
	{
		if (onVI)
		{
			if (SimTime() - lastSwitchVI >= onPeriodVI)
			{
				onVI = false;
				on_off_VI.Set(SimTime() + offPeriodVI);
				lastSwitchVI = SimTime ();
			}else
			{
				source_VI.Set(SimTime ());
				on_off_VI.Set(SimTime () + onPeriodVI);
			}
		}else
		{
			if (SimTime () - lastSwitchVO >= offPeriodVO)
			{
				onVI = true;
				source_VI.Set(SimTime());
				on_off_VI.Set(SimTime() + onPeriodVI);
			}
		}
	}else
	{
		source_VI.Set(SimTime ());
	}
}

void BatchPoissonSource :: new_VI_packet(trigger_t &)
{
	Packet pkt;
	//Average of the ones tested for h264/AVC with a variable bitrate adaptaion algorithm
	double vi_rate = 16e6 / ((Iav + Pav + Bav) / 3);
	if (onVI)
	{
		pkt.L = pickFrameFromH264Gop (gopPos);
		cout << pkt.L << endl;
		pkt.accessCategory = 2;
		out(pkt);
		registerStatistics (pkt);
		source_VI.Set(SimTime () + Exponential (1/vi_rate));
	}

}
void BatchPoissonSource :: new_BE_packet(trigger_t &)
{
	Packet pkt;
	pkt.accessCategory = 1;
	out(pkt);
	registerStatistics (pkt);
	source_BE.Set(SimTime () + Exponential (1/packet_rate));
}
void BatchPoissonSource :: new_BK_packet(trigger_t &)
{
	Packet pkt;
	pkt.accessCategory = 0;
	out(pkt);
	registerStatistics (pkt);
	source_BK.Set(SimTime () + Exponential (1/packet_rate));
}

// Old implementation without G.711 and H.264/AVC configuration
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
	out(packet);
	packetsGenerated += 1;
	packetsInAC.at(packet.accessCategory) += 1;
	lambda = RB/packet_rate;
	if(packetsGenerated > MAXSEQ){
		if(alwaysSat == 1){
			lambda = 1;
		}
	}
	inter_packet_timer.Set(SimTime()+Exponential(lambda));	
};

