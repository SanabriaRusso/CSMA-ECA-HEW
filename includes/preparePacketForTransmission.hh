#include <math.h>
#include <algorithm>

#define AC 4
//refer to Channel.h and rate = 1000
#define MacDel 240
#define MacHead 32
#define maxViFrame 18500
#define maxVoFrame 6150

Packet preparePacketForTransmission(int acToTx, double txTime, std::array<Packet,AC> &superPacket, 
	int id, std::array<int,AC> &stages, std::array<FIFO <Packet>, AC> &Queues, int fairShare, int ECA, bool &TXOP)
{
	int viFrames = 0;
	int voFrames = 0;
	long int load = 0;

	superPacket.at(acToTx).source = id;
	superPacket.at(acToTx).tx_time = txTime;
	superPacket.at(acToTx).accessCategory = acToTx;
	superPacket.at(acToTx).QoS = 0;

	if(fairShare == 1)
	{		
		superPacket.at(acToTx).aggregation = std::min( (int)pow(2,stages.at(acToTx)), 
			Queues.at(acToTx).QueueSize() );
		superPacket.at(acToTx).QoS = 1;

		if (TXOP)
		{
			switch (acToTx)
			{
				case 2:
					while (load < maxViFrame)
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
					superPacket.at(acToTx).aggregation = std::min(viFrames, Queues.at(acToTx).QueueSize() );
					break;
				case 3:
					while (load < maxVoFrame)
					{
						if (Queues.at(superPacket.at(acToTx).accessCategory).QueueSize() > voFrames)
						{
							load += Queues.at(superPacket.at(acToTx).accessCategory).GetPacket(voFrames).L + MacDel + MacHead;
							voFrames ++;
						}else
						{
							break;
						}
					}
					if ((voFrames > 1) && (load > maxVoFrame)) voFrames --;
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
	}
	superPacket.at(acToTx).L = load;

	// cout << "-AC: " << acToTx << endl;
	// cout << "\tSending: " << superPacket.at(acToTx).aggregation << endl;

	return((Packet)(superPacket.at(acToTx)));
}