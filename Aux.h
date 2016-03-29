#ifndef _AUX_
#define _AUX_

struct Packet
{
	int source;
	int destination;
	long int L;
	long int seq; //current packet seq
	std::vector<long int> allSeq; //used with FairShare
	double tx_time; //time at which a packet is transmitted
	double queuing_time; //time at which the packet is deposited at end of the queue
	double contention_time;
	int fairShare; // 0 = not activated. 1 = ECA
	int aggregation; //used whenever fairShare is activated
	int accessCategory; // 0 = best-effort, 1 = background, 2 = video, 3 = voice
	int startContentionStage;
	//set in STA :: preparePacketForTransmission
	int QoS; // 1 if variable frame size, to avoid over estimating in Channel.h
	int firstMPDUSeq;
	int lastMPDUSeq;
};

struct SLOT_notification
{
	int status; 			// 0 = empty, 1 = succ. tx, >1 = collision
	signed long long num;	// slot number
	int error;				// 0 = noError, other = number of packets affected by the error
	std::vector<int> affectedFrames; // which frames were affected. Simulating aMPDU
};


#endif 

