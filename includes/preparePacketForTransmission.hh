#include <math.h>
#include <algorithm>

#define AC 4
//refer to Channel.h and rate = 1000
#define MacDel 240
#define MacHead 32

Packet preparePacketForTransmission(int acToTx, double txTime, std::array<Packet,AC> &superPacket, 
	int id, std::array<int,AC> &stages, std::array<FIFO <Packet>, AC> &Queues, int fairShare, int ECA, bool &TXOP)
{
	int viFrames = 0;
	int voFrames = 0;
	long int load = 0;
	bool forceFairShare = false;
	int maxViFrame = 18500;
	int maxVoFrame = 6150;
	if (forceFairShare)
		maxVoFrame = maxViFrame;

	superPacket.at(acToTx).source = id;
	superPacket.at(acToTx).tx_time = txTime;
	superPacket.at(acToTx).accessCategory = acToTx;
	superPacket.at(acToTx).QoS = 0;

	if(fairShare == 1)
	{
		superPacket.at(acToTx).aggregation = std::min(1, Queues.at(acToTx).QueueSize() );
		if (acToTx > 1)		
			superPacket.at(acToTx).aggregation = std::min( (int)pow(2,stages.at(acToTx)), 
				Queues.at(acToTx).QueueSize() );
		superPacket.at(acToTx).QoS = 1;
		if (TXOP)
		{
			switch (acToTx)
			{
				case 2:
					while (load <= maxViFrame)
					{
						if (Queues.at(acToTx).QueueSize() > viFrames)
						{
							load += Queues.at(superPacket.at(acToTx).accessCategory).GetPacket(viFrames).L + MacDel + MacHead;
							viFrames ++;
						}else
						{
							break;
						}
					}
					if ((viFrames > 1) && (load > maxViFrame)) viFrames --;
					if (ECA == 1) viFrames = std::min(viFrames, superPacket.at(acToTx).aggregation);
					if (forceFairShare)
						assert (viFrames = std::min((int)pow(2,stages.at(acToTx)), Queues.at(acToTx).QueueSize()));
					superPacket.at(acToTx).aggregation = std::min(viFrames, Queues.at(acToTx).QueueSize() );
					break;
				case 3:
					while (load <= maxVoFrame)
					{
						if (Queues.at(acToTx).QueueSize() > voFrames)
						{
							load += Queues.at(superPacket.at(acToTx).accessCategory).GetPacket(voFrames).L + MacDel + MacHead;
							// cout << "frame: -" << voFrames << ", size: " << Queues.at(superPacket.at(acToTx).accessCategory).GetPacket(voFrames).L << endl;
							voFrames ++;

						}else
						{
							break;
						}
					}
					if ((voFrames > 1) && (load > maxVoFrame)) voFrames --;
					if (ECA == 1) voFrames = std::min(voFrames, superPacket.at(acToTx).aggregation);
					if (forceFairShare)
						assert (voFrames = std::min((int)pow(2,stages.at(acToTx)), Queues.at(acToTx).QueueSize()));
					superPacket.at(acToTx).aggregation = std::min(voFrames, Queues.at(acToTx).QueueSize() );
					break;
				default:
					superPacket.at(acToTx).aggregation = 1;
					break;
			}
		}
	}else
	{
		superPacket.at(acToTx).aggregation = 1;
	}

	int limit = superPacket.at(acToTx).aggregation;
	superPacket.at(acToTx).allSeq.assign (limit,0);
	load = 0;
	for(int i = 0; i < limit; i++)
	{	
		load += Queues.at(superPacket.at(acToTx).accessCategory).GetPacket(i).L;
		superPacket.at(acToTx).allSeq.at (i) = Queues.at(superPacket.at(acToTx).accessCategory).GetPacket(i).seq;
		if (i == 0)
			superPacket.at(acToTx).firstMPDUSeq = superPacket.at(acToTx).allSeq.at (i);
		if (i == limit -1)
			superPacket.at(acToTx).lastMPDUSeq = superPacket.at(acToTx).allSeq.at (i);
	}
	superPacket.at(acToTx).L = load;

	return((Packet)(superPacket.at(acToTx)));
}