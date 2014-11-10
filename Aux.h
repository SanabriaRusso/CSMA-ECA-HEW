#ifndef _AUX_
#define _AUX_

struct Packet
{
	int source;
	int destination;
	int L;
	int seq;
	double tx_time; //time at which a packet is transmitted
	double queuing_time; //time at which the packet is deposited at end of the queue
	double contention_time;
	int aggregation; //used whenever fairShare is activated
	int accessCategory; // 0 = best-effort, 1 = background, 2 = video, 3 = voice
};

struct SLOT_notification
{
	int status; // 0 = empty, 1 = succ. tx, >1 = collision
};


#endif 

