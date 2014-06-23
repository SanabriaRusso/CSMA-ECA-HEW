#ifndef _AUX_
#define _AUX_

struct Packet
{
	int source;
	int destination;
	int L;
	int seq;
	double send_time; //time at which a packet is picked from the MAC queue
	double queuing_time; //time at which the packet is deposited at end of the queue
	int aggregation; //used whenever fairShare is activated
	int accessCategory; // 0 = best-effort, 1 = background, 2 = video, 3 = voice
};

struct SLOT_notification
{
	int status; // 0 = empty, 1 = succ. tx, >1 = collision
};


#endif 

