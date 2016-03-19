long int getPayloadForTxDuration (Packet &packet, std::array<FIFO <Packet>, AC> &Queues)
{
	int limit = packet.aggregation;
	packet.allSeq.assign (limit,0);
	long int load = 0;
	for(int i = 0; i < limit; i++)
	{	
		load += Queues.at(packet.accessCategory).GetPacket(i).L;
		packet.allSeq.at (i) = Queues.at(packet.accessCategory).GetPacket(i).seq;
	}
	// cout << "Ac-" << packet.accessCategory << ": fairShare: " << limit << ": load: " << load << endl;
	return load;
}