#ifndef _FIFO_QUEUE_
#define _FIFO_QUEUE_


//#include "Aux.h"
#include <deque>


/*
	FIFO Class
*/

template <class DATATYPE> class FIFO 
{	
	private:
		std::deque <DATATYPE> m_queue;
		
	public:
		DATATYPE GetFirstPacket();
		void DelFirstPacket();		
		void PutPacket(DATATYPE &packet);	
		int QueueSize();
		DATATYPE GetPacket(int &i);
		void PushFront(FIFO <DATATYPE> &Q);
};

template <class DATATYPE> DATATYPE FIFO <DATATYPE> :: GetFirstPacket()
{
	return(m_queue.front());	
}; 

template <class DATATYPE> void FIFO <DATATYPE> :: DelFirstPacket()
{
	if(m_queue.size() == 0)
	{
		printf("Attempting to erase an empty queue\n");	
	}else
	{
		m_queue.pop_front();
	}
}; 

template <class DATATYPE> void FIFO <DATATYPE> :: PutPacket(DATATYPE &packet)
{	
	m_queue.push_back(packet);
}; 

template <class DATATYPE> int FIFO <DATATYPE> :: QueueSize()
{
	return(m_queue.size());
}; 

template <class DATATYPE> DATATYPE FIFO <DATATYPE> :: GetPacket(int &i)
{
	return(m_queue.at(i));
};

template <class DATATYPE> void FIFO <DATATYPE> :: PushFront(FIFO <DATATYPE> &Q)
{
	int init = m_queue.size ();
	int qSize = Q.QueueSize ();
	for (int i = 0; i < qSize; i++)
	{
		m_queue.push_front(Q.GetFirstPacket());
		Q.DelFirstPacket ();
	}
	assert(init + qSize == m_queue.size ());
};

// If required, there are also functions like "put_the_packet_at_position_X" or "delete_packet_placed_in_position_X"

#endif
