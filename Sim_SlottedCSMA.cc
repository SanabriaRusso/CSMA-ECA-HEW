#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <iostream>
#include <fstream>

#include "./COST/cost.h"

#include <deque>

#include "Channel.h"
#include "STA.h"
// #include "BatchPoissonSource.h"
#include "BatchPoissonSource_enhanced.h"
#include "stats/stats.h"

using namespace std;

component SlottedCSMA : public CostSimEng
{
	public:
		void Setup(int Sim_Id, int NumNodes, int PacketLength, double Bandwidth, int Batch, int Stickiness, int ECA, int fairShare, float channelErrors, float slotDrift,float percentageDCF, int maxAggregation, int simSeed);
		void Stop();
		void Start();		

	public:
		Channel channel;
//		SatNode [] stas;
		STA [] stas;
		BatchPoissonSource [] sources;

	private:
		int SimId;
		int Nodes;
		double Bandwidth_;
		int PacketLength_;
		int Batch_;
		float drift;
		double intCut, decimalCut, cut; 
		

};

void SlottedCSMA :: Setup(int Sim_Id, int NumNodes, int PacketLength, double Bandwidth, int Batch, int Stickiness, int ECA, int fairShare, float channelErrors, float slotDrift, float percentageEDCA, int maxAggregation, int simSeed)
{
	SimId = Sim_Id;
	Nodes = NumNodes;
	drift = slotDrift;

	stas.SetSize(NumNodes);
	sources.SetSize(NumNodes);

	// Channel	
	channel.Nodes = NumNodes;
	channel.out_slot.SetSize(NumNodes);
	channel.error = channelErrors;

	// Sat Nodes
	//Determining the cut value for assigning different protocols
	cut = NumNodes * percentageEDCA;
	decimalCut = modf(cut, &intCut);
	
	if(decimalCut > 0.5)
	{
		intCut++;	
	}
	
	for(int n=0;n<NumNodes;n++)
	{
		// Node
		stas[n].node_id = n;
		stas[n].K = 1000;
		stas[n].system_stickiness = Stickiness;
		stas[n].ECA = ECA;
		stas[n].fairShare = fairShare;
		//stas[n].driftProbability = slotDrift;
		stas[n].cut = intCut;     
		stas[n].maxAggregation = maxAggregation;
		stas[n].L = PacketLength;


		// Traffic Source
		sources[n].L = PacketLength;
		
		// Implementing a faster source
		sources[n].packet_rate = Bandwidth/(PacketLength * 8);		//Global source packet generation rate

		// The percentage of generated packets destined to a specific AC
		sources[n].BEShare = 100; // 35%
		sources[n].BKShare = 65;  // 35%
		sources[n].VIShare = 35;  // 25%
		sources[n].VOShare = 5;  // 5%

		// sources[n].packet_rateBE = Bandwidth/(PacketLength * 8);
		// sources[n].packet_rateBK = Bandwidth/(PacketLength * 8);
		// sources[n].packet_rateVI = (Bandwidth/8)/(PacketLength * 8);
		// sources[n].packet_rateVO = (Bandwidth/32)/(PacketLength * 8);
		
		// sources[n].packet_rateBE = 0;
		// sources[n].packet_rateBK = 0;
		// sources[n].packet_rateVI = 0;
		// sources[n].packet_rateVO = 0;

		sources[n].MaxBatch = Batch;
	}
	
	// Connections
	for(int n=0;n<NumNodes;n++)
	{
        connect channel.out_slot[n],stas[n].in_slot;
		connect stas[n].out_packet,channel.in_packet;
		connect sources[n].out,stas[n].in_packet;
	}


	Bandwidth_ = Bandwidth;
	PacketLength_ = PacketLength;
	Batch_ = Batch;
		

};

void SlottedCSMA :: Start()
{
	cout << ("--------------- Starting ---------------") << endl;
};

void SlottedCSMA :: Stop()
{
	
	//--------------------------------------------------------------//
	//-------------------Writing the results------------------------//
	//--------------------------------------------------------------//

	//CLI output variables
	array <double,AC> totalACthroughput = {};
	double totalThroughput = 0.0;

	array <double,AC> overallSxTx = {};
	double totalSentPackets = 0.0;

	array <double,AC> overallTx = {};
	double totalTx = 0.0;
	array <double,AC> totalACRet = {};
	double totalRetransmissions = 0.0;

	array <double,AC> totalACCol = {};
	double totalCol = channel.collision_slots;
	double sumOfCol = 0.0;

	array <double,AC> totalIntACCol = {};
	double totalIntCol = 0.0;

	array <double,AC> droppedAC = {};
	double totalDropped = 0.0;

	double totalIncommingPackets = 0.0;
	double totalErasedPackets = 0.0;
	double totalRemainingPackets = 0.0;

	array <double,AC> fairnessAC = {};
	array <double,AC> fairnessACNum = {};
	array <double,AC> fairnessACDenom = {};
	double overallFairnessNum = 0.0;
	double overallFairnessDenom = 0.0;

	array <double,AC> avgTimeBetweenACSxTx = {};

	array <double,AC> qEmpties = {};


	for (int i = 0; i < Nodes; i++){
		for (int j = 0; j < AC; j++){
			totalThroughput += stas[i].overallACThroughput.at(j);
			totalACthroughput.at(j) += stas[i].overallACThroughput.at(j);

			totalSentPackets += (stas[i].packetsSent.at(j));
			overallSxTx.at(j) += (stas[i].packetsSent.at(j));
			
			totalTx += (stas[i].transmissions.at(j));
			overallTx.at(j) += (stas[i].transmissions.at(j));

			totalRetransmissions += (stas[i].totalACRet.at(j));
			totalACRet.at(j) += (stas[i].totalACRet.at(j));

			sumOfCol += (stas[i].totalACCollisions.at(j));
			totalACCol.at(j) += (stas[i].totalACCollisions.at(j));

			totalIntCol += (stas[i].totalInternalACCol.at(j));
			totalIntACCol.at(j) += (stas[i].totalInternalACCol.at(j));

			totalDropped += (stas[i].droppedAC.at(j));
			droppedAC.at(j) += (stas[i].droppedAC.at(j));

			fairnessACNum.at(j) += (stas[i].overallACThroughput.at(j));
			overallFairnessNum += (stas[i].overallACThroughput.at(j));
			fairnessACDenom.at(j) += (double)pow(stas[i].overallACThroughput.at(j), 2);
			overallFairnessDenom += (double)pow(stas[i].overallACThroughput.at(j), 2);

			avgTimeBetweenACSxTx.at(j) += (stas[i].accumTimeBetweenSxTx.at(j) / stas[i].sxTx.at(j));

			qEmpties.at(j) += (stas[i].queueEmpties.at(j));
		}
		totalIncommingPackets += (stas[i].incommingPackets);
		totalErasedPackets += (stas[i].erased);
		totalRemainingPackets += (stas[i].remaining);

	}

	ofstream file;
	file.open("Results/output.txt", ios::app);
	file << "#1. Nodes 					2. totalThroughput 			3. totalBEThroughput 		4. totalBKThroughput "<< endl;
	file << "#5. totalVIThroughput 		6. totalVOThroughput 		7. totalCollisionsSlots 	8. fractionBECollisions "<< endl;
	file << "#9. fractionBKCollisions	10. fractionVICollisions 	11. fractionVOCollisions 	12. totalInternalCollisions" << endl;
	file << "#13. totalBEIntCol 		14. totalBKIntCol 			15. totalVIIntCol 	 		16. totalVOIntCol" << endl;
	file << "#17. overallFairness 		18. BEFairness				19. BKFairness				20. VIFairness"	<< endl;
	file << "#21. VOFairness			22. avgTimeBtSxTxBE			23. avgTimeBtSxTxBK			24. avgTimeBtSxTxVI" << endl;
	file << "#25. avgTimeBtSxTxVO		26. qEmptyBE				27. qEmptyBK				28. qEmptyVI" << endl;
	file << "#29. qEmptyVO				30. totalDropped			31. droppedBE				32. droppedBK" << endl;
	file << "#33. droppedVI				34. droppedVO" << endl;
	
	file << Nodes << " " << totalThroughput << " ";
	//Printing AC related metrics
	//3 - 6
	for (int i = 0; i < AC; i++){
		file << totalACthroughput.at(i) << " ";
	}

	//7-11
	file << totalCol << " ";
	for (int i = 0; i < AC; i++){
		file << (double)(totalACCol.at(i)/totalTx) << " ";
	}

	//12-16
	file << totalIntCol << " ";
	for (int i = 0; i < AC; i++){
		file << totalIntACCol.at(i) << " ";
	}

	//17-21
	file << (double) ( (pow(overallFairnessNum,2)) / (Nodes * (overallFairnessDenom)) ) << " ";
	for (int i = 0; i < AC; i++){
		fairnessAC.at(i) = (double) ( (pow(fairnessACNum.at(i),2)) / (Nodes * (fairnessACDenom.at(i))) );
		if(fairnessAC.at(i) > 0)
		{
			file << fairnessAC.at(i) << " ";
		}else
		{
			file << "0 ";
			fairnessAC.at(i) = 0.0;
		}
	}

	//22-25
	for(int i = 0; i < AC; i++){
		if(avgTimeBetweenACSxTx.at(i) > 0)
		{
			file << avgTimeBetweenACSxTx.at(i)/Nodes << " ";
		}else
		{
			file << "0 ";
			avgTimeBetweenACSxTx.at(i) = 0.0;
		}
	}

	//26-29
	for(int i = 0; i < AC; i++){
		if(qEmpties.at(i) > 0)
		{
			file << qEmpties.at(i) << " ";
		}else
		{
			file << "0 ";
			qEmpties.at(i) = 0;
		}
	}

	//30-34
	if(totalDropped > 0)
	{
		file << totalDropped << " ";
	}else
	{
		file << "0 ";
		totalDropped = 0;
	}
	for(int i = 0; i < AC; i++)
	{
		if(droppedAC.at(i) > 0)
		{
			file << droppedAC.at(i) << " ";
		}else
		{
			file << "0 ";
			droppedAC.at(i) = 0;
		}
	}

	file << endl;


	//--------------------------------------------------------------//
	//---------Presentation when simulation ends--------------------//
	//--------------------------------------------------------------//
	



	cout << endl;
	cout << "---------------------------"<< endl;
	cout << "--- Overall Statistics ----" << endl;
	cout << "---------------------------"<< endl;

	cout << "1. Total transmissions: " << totalTx << ". Total Throughput (Mbps): " << totalThroughput << endl;
	cout << "1.1 Packet generation: " << Bandwidth_ << endl;
	for(int i = 0; i < AC; i++)
	{
		cout << "\tAC " << i << ": " << overallTx.at(i) << ". Total Throughput for AC (Mbps): " 
			<< totalACthroughput.at(i) << endl;;

	}

	cout << "\n2. Total Collisions: " << totalCol << endl;
	cout << "2.1 Total Internal Collisions: " << totalIntCol << ". Internal Collisions + Collisions: "
		<< totalIntCol + totalCol << endl;
	cout << "2.2 Summing collision metrics from all stations" << endl;
	for(int i = 0; i < AC; i++)
	{
		cout << "\tAC " << i << ": " << totalACCol.at(i) << ". Collisions AC / Total Transmitted: " 
			<< totalACCol.at(i) / totalTx << endl;
		cout << "\tInternal Collisions: " << totalIntACCol.at(i) << ". Internal Collisions AC + Collisions AC: "
			<< totalIntACCol.at(i) + totalACCol.at(i) << endl << endl;

	}

	cout << "\n3. Total Retransmissions: " << totalRetransmissions << ". Retransmitted / Transmitted ratio: " 
		<< totalRetransmissions / totalTx << endl;

	for(int i = 0; i < AC; i++)
	{
		cout << "\tAC " << i << ": " << totalACRet.at(i) << ". Retransmitted AC / Total Transmitted: " 
			<< totalACRet.at(i) / totalTx << endl;

	}

	cout << "\n4. Total Dropped packets due to RET " << totalDropped << ". Dropped / SxSent ratio: "
		<< totalDropped / totalSentPackets << endl;

	for(int i = 0; i < AC; i++)
	{
		cout << "\tAC " << i << ": " << droppedAC.at(i) << ". Dropped AC / SxSent ratio: "
			<< droppedAC.at(i) / totalSentPackets << endl;
	}

	cout << "\n5. Overall erased packets failure index (at least one should be 1). In sat: " << 
	( (totalIncommingPackets - totalErasedPackets) / totalRemainingPackets ) << ". Non-sat: " << 
	totalIncommingPackets / totalErasedPackets <<  endl;


	cout << "\n6. Overal Fairness: " << (double)((pow(overallFairnessNum,2))/(Nodes*(overallFairnessDenom))) << endl;

	for(int i = 0; i < AC; i++)
	{
		cout << "\tAC " << i << " fairness: " << fairnessAC.at(i) << endl;
	}

	cout << "\n7. Average time between successful transmissions for each AC:" << endl;

	for(int i = 0; i < AC; i++)
	{
		cout << "\tAC " << i << ": " << avgTimeBetweenACSxTx.at(i)/Nodes << endl;
	}

	cout << "\n8. Number of times the queue empties for each AC:" << endl;

	for(int i = 0; i < AC; i++)
	{
		cout << "\tAC " << i << ": " << qEmpties.at(i) << endl;
	}

};

int main(int argc, char *argv[])
{
	int MaxSimIter;
	double SimTime;
	int NumNodes;
	int PacketLength;
	double Bandwidth;
	int Batch;
	int Stickiness;
	int ECA;
	int fairShare;
	float channelErrors;
	float slotDrift;
	float percentageEDCA;
	int maxAggregation;
	int simSeed;
	
	if(argc < 12) 
	{
		if(argv[1])
		{
			string word = argv[1];
			string help ("--help");
			string helpShort ("-h");
			if((word.compare(help) == 0) || (word.compare(helpShort) == 0)){
				cout << endl;
				cout << "------------" << endl;
				cout << "Cheatsheet:" << endl;
				cout << "------------" << endl;
				cout << "(0)./XXXX (1)SimTime (2)NumNodes (3)PacketLength (4)Bandwidth (5)Batch (6)ECA (7)Stickiness (8)fairShare (9)channelErrors (10)slotDrift (11)percentageOfEDCA (12)maxAggregation (13)simSeed" << endl << endl;;
				cout << "0) ./XXX: Name of executable file" << endl;
				cout << "1) SimTime: simulation time in seconds" << endl;
				cout << "2) NumNodes: number of contenders" << endl;
				cout << "3) PacketLength: length of the packet in bytes" << endl;
				cout << "4) Bandwidth: number of bits per second generated by the source. With 802.11n and EDCA, 10e6 < is considered an unsaturated environment." << endl;
				cout << "5) Batch: how many packets are put in the contenders queue. Used to simulate burst traffic. Usually set to 1" << endl;
				cout << "6) EDCA: using a deterministic backoff after successful transmissinos (0=no, 1=yes)" << endl;
				cout << "7) Stickiness: a deterministic backoff is used for this many number of collisions" << endl;
				cout << "8) FairShare: nodes at backoff stage k, attempt the transmission of 2^k packets (0=off, 1=on)" << endl;
				cout << "9) ChannelErrors: channel errors probability [0,1]" << endl;
				cout << "10) SlotDrift: probability of miscounting passing empty slots [0,1]" << endl;
				cout << "11) PercetageEDCA: percentage of nodes running EDCA. Used to simulate CSMA/ECA and CSMA/CA mixed scenarios [0,1]" << endl;
				cout << "12) MaxAggregation: nodes use maximum aggregation when attempting transmission (0=off, 1=on)" << endl;
				cout << "13) SimSeed: simulation seed used to generate random numbers. If testing results, repeat simulations with different seeds everytime" << endl << endl;
				return(0);
			}else
			{
				cout << endl;
				cout << "Alert: Unintelligible command" << endl;
				cout << "Use the parameter --help or -h to display the available settings" << endl << endl;
				return(0);
			}
		}else
		{
			cout << "Executed with default values shown below" << endl;
			cout << "./XXXX SimTime [10] NumNodes [2] PacketLength [1024] Bandwidth [65e6] Batch [1] ECA [0] Stickiness [0] fairShare [0] channelErrors [0] slotDrift [0] percentageOfEDCA [1] maxAggregation [0] simSeed [0]" << endl;
			MaxSimIter = 1;
			SimTime = 10;
			NumNodes = 30;
			PacketLength = 1024;
			Bandwidth = 65e6;
			Batch = 1; // =1
			Stickiness = 0; // 0 = EDCA, up to 2.
			ECA = 0;	//0 = EDCA, 1 = ECA
			fairShare = 0; //0 = EDCA, 1 = CSMA-ECA
			channelErrors = 0; // float 0-1
			slotDrift = 0; // // float 0-1
			percentageEDCA = 1; // // float 0-1
			maxAggregation = 0;
			simSeed = 2234; //Simulation seed
		}
	}else
	{
		MaxSimIter = 1;
		SimTime = atof(argv[1]);
		NumNodes = atoi(argv[2]);
		PacketLength = atoi(argv[3]);
		Bandwidth = atof(argv[4]);
		Batch = atoi(argv[5]); // =1
		ECA = atoi(argv[6]); //0 = EDCA, 1 = CSMA-ECA
		Stickiness = atoi(argv[7]); // 0 = EDCA.
		fairShare = atoi(argv[8]); //0 = EDCA, 1 = CSMA-ECA
		channelErrors = atof(argv[9]); // float 0-1
		slotDrift = atof(argv[10]); // // float 0-1
		percentageEDCA = atof(argv[11]); // // float 0-1
		maxAggregation = atoi(argv[12]); //0 = no, 1 = yes
		simSeed = atof(argv[13]); //Simulation seed
	}

	printf("\n####################### Simulation (Seed: %d) #######################\n",simSeed);
	if(Stickiness > 0)
	{
		if(ECA > 0)
		{
			if(fairShare > 0)
			{
				cout << "####################### Full ECA #######################" << endl;
			}else
			{
				cout << "################### ECA + hysteresis ###################" << endl;
			}
		}else
		{
			cout << "###################### Basic ECA ######################" << endl;
		}
	}else
	{
		cout << "####################### CSMA/CA #######################" << endl;
	}
	
	if(percentageEDCA > 0) cout << "####################### Mixed setup " << percentageEDCA*100 << "% EDCA #######################" << endl;
		
	SlottedCSMA test;

	//test.Seed=(long int)6*rand();
	
	test.Seed = simSeed;
		
	test.StopTime(SimTime);

	test.Setup(MaxSimIter,NumNodes,PacketLength,Bandwidth,Batch,Stickiness, ECA, fairShare, channelErrors, slotDrift, percentageEDCA, maxAggregation, simSeed);
	
	test.Run();


	return(0);
};
