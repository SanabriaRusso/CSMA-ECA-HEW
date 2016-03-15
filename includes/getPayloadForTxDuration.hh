long int getPayloadForTxDuration (Packet &packet, std::array<FIFO <Packet>, AC> &Queues)
{
	int limit = packet.aggregation;
	long int load = 0;
	for(int i = 0; i < limit; i++)
	{	
		load += Queues.at(packet.accessCategory).GetPacket(i).L;
	}
	return load;
}