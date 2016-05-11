/*
	Poisson source. 
*/
			
#include "Aux.h"

#define MAXSEQ 1000
#define AC 4

//Rate of G.723.1=6.4kbps, G.711=64kbps, iLBC=15.2kbps
#define VAF 0.448 //Voice Activity Factor

//For G.723.1
// #define avgVoON 11.54
// #define avgVoOFF 11.98
//For iLBC
#define avgVoON 3.113
#define avgVoOFF 3.279
//
#define epsilon 1e-3
/*
 * Defining traffic source characteristics for CIF enconding of 720p source
 *
 * a 16 frames GOP, with 3 B frames per I/P frames (G16-B3), H.264/AVC 
 */
#define GOPSIZE 16
/*Average frame sizes (Bytes) for a PSNR of 43.5dB, average bitrate 1e6 bits/s*/
#define Iav 5658
#define Pav 1634
#define Bav 348
//Average covariance for the frame size at specified PSNR.
#define CoV 2 //not used, but means that frames can double their average size
#define avgViRate 3e5 //again, matching the PSNR
// IBBBPBBBPBBBPBBB
extern "C" const double gop [GOPSIZE] = {Iav,Bav,Bav,Bav,Pav,Bav,Bav,Bav,Pav,Bav,Bav,Bav,Pav,Bav,Bav,Bav};

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
		long int seq, voSeq, viSeq, beSeq, bkSeq;
		int MaxBatch;	
		double packet_rate;
		double vo_rate, vi_rate, bandwidthVO;
		double voPayload;
		double voPacketInterval;
		int categories;
		int packetGeneration;
		double packetsGenerated;
		bool saturated;
		bool sameSizeFrames;
		bool QoS;
		bool changingFrameSize;
		std::array<double,AC> packetsInAC;
		double SATMULTIPLIER;

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
	double frameSizeCoefficient = 1;
	size = gop[pos];
	if (changingFrameSize)
		frameSizeCoefficient = ((rand() % (int) 100.0)/100.0) + 1.0;
	size *= sqrt(frameSizeCoefficient);

	return size;
}

void BatchPoissonSource :: Setup()
{

};

void BatchPoissonSource :: Start()
{
	QoS = true;
	saturated = true;






	changingFrameSize = true; //randomising frame sizes for AC[VI] according to the std
	sameSizeFrames = false;
	if (saturated)
	{
		changingFrameSize = false;
		sameSizeFrames = true;
	}


	SATMULTIPLIER = 1;
	if (saturated)
		SATMULTIPLIER = 0.001;

	//Determining on and off periods according to a Geom-APD-W2, but with VAF from W0
	onPeriodVO = (VAF * (avgVoON + avgVoOFF)) * 1.0;
	offPeriodVO = ((1 - VAF) * (avgVoON + avgVoOFF)) * 1.0;
	if (saturated) onPeriodVO = 0;

	onPeriodVI = 0; //on-off in video is off by default.
	offPeriodVI = 0.1;
	onVO = true;
	onVI = true;

	//Defined by G.723.1 size 24B, interval 30e-3, and on/off values
	//Defined Internet Low Bit Rate Codec (iLBC) size 38B, interval 20e-3, and on/off values
	//Define by G.711 frame size 80B, interval 10e-3, CBR
	voPayload = 38 * 8;
	voPacketInterval = 20e-3;
	vo_rate = (voPayload / voPacketInterval) / voPayload;
	//Average of the ones tested for h264/AVC with a variable bitrate adaptaion algorithm
	vi_rate = avgViRate / (Iav*8);

	seq = 0;
	voSeq = 0;
	viSeq = 0;
	beSeq = 0;
	bkSeq = 0;
	gopPos = 0;

	if (!saturated)
	{
		if (QoS)
		{
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
	}else
	{
		switch (categories)
		{
			case 1:
				source_VO.Set(SimTime ());
				break;
			default:
				source_VO.Set(SimTime ());
				source_VI.Set(SimTime ());
				source_BE.Set(SimTime ());
				source_BK.Set(SimTime ());
				break;
		}
	}
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
			if (SimTime () - lastSwitchVO >= offPeriodVO - epsilon)
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
		pkt.L = voPayload / 8;
		if (sameSizeFrames)
			pkt.L = L;
		pkt.seq = seq;
		seq ++;
		voSeq ++;
		out (pkt);
		registerStatistics (pkt);
		double lambda = (1.0 / vo_rate) * SATMULTIPLIER;
		double nextVoPacket = SimTime() + Exponential(lambda);
		
		if (!(saturated && voSeq == MAXSEQ))
			source_VO.Set(nextVoPacket);
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
	long int size;
	if (onVI)
	{
		size = pickFrameFromH264Gop (gopPos);
		pkt.L = size;
		if (sameSizeFrames)
			pkt.L = L;
		pkt.accessCategory = 2;
		pkt.seq = seq;
		seq ++;
		viSeq ++;
		out(pkt);

		gopPos++;
		if (gopPos == GOPSIZE) gopPos = 0;
		registerStatistics (pkt);
		vi_rate = avgViRate / (pkt.L * 8);
		double lambda = (1.0 / vi_rate) * SATMULTIPLIER;
		double nextViPacket = SimTime () + Exponential (lambda);
		if (!(saturated && viSeq == MAXSEQ))
			source_VI.Set(nextViPacket);
	}
}
void BatchPoissonSource :: new_BE_packet(trigger_t &)
{
	Packet pkt;
	pkt.accessCategory = 1;
	pkt.L = L;
	pkt.seq = seq;
	seq ++;
	beSeq ++;
	out(pkt);
	registerStatistics (pkt);
	double lambda = (1 / packet_rate) * SATMULTIPLIER;
	double nextBePacket = SimTime () + Exponential (lambda);
	if (!(saturated && beSeq == MAXSEQ))
		source_BE.Set(nextBePacket);

}
void BatchPoissonSource :: new_BK_packet(trigger_t &)
{
	Packet pkt;
	pkt.accessCategory = 0;
	pkt.L = L;
	pkt.seq = seq;
	seq ++;
	bkSeq ++;
	out(pkt);
	registerStatistics (pkt);
	double lambda = (1 / packet_rate) * SATMULTIPLIER;
	double nextBkPacket = SimTime () + Exponential (lambda);
	if (!(saturated && bkSeq == MAXSEQ))
		source_BK.Set(nextBkPacket);
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
	packet.seq = seq;
	seq ++;
	out(packet);
	packetsGenerated += 1;
	packetsInAC.at(packet.accessCategory) += 1;
	double lambda = RB/packet_rate;
	inter_packet_timer.Set(SimTime()+Exponential(lambda));	
};

