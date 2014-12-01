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
#include "BatchPoissonSource.h"
#include "stats/stats.h"

using namespace std;

component SlottedCSMA : public CostSimEng
{
	public:
		void Setup(int Sim_Id, int NumNodes, int PacketLength, double Bandwidth, int Batch, int Stickiness, int hysteresis, int fairShare, float channelErrors, float slotDrift,float percentageDCF, int maxAggregation, int simSeed);
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

void SlottedCSMA :: Setup(int Sim_Id, int NumNodes, int PacketLength, double Bandwidth, int Batch, int Stickiness, int hysteresis, int fairShare, float channelErrors, float slotDrift, float percentageEDCA, int maxAggregation, int simSeed)
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
		stas[n].hysteresis = hysteresis;
		stas[n].fairShare = fairShare;
		//stas[n].driftProbability = slotDrift;
		stas[n].cut = intCut;     
		stas[n].maxAggregation = maxAggregation;
		stas[n].L = PacketLength;


		// Traffic Source
		sources[n].L = PacketLength;
		sources[n].packet_rateBE = Bandwidth/PacketLength;
		sources[n].packet_rateBK = Bandwidth/PacketLength;
		sources[n].packet_rateVI = (Bandwidth/2)/PacketLength;
		sources[n].packet_rateVO = (Bandwidth/4)/PacketLength;
		/*
		sources[n].packet_rateBK = Bandwidth/PacketLength;
		
		sources[n].packet_rateVO = (Bandwidth/4)/PacketLength;*/

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
	double totalSxTx = 0.0;

	array <double,AC> overallTx = {};
	double totalTx = 0.0;
	array <double,AC> totalACRet = {};
	double totalRetransmissions = 0.0;

	array <double,AC> totalACCol = {};
	double totalCol = 0.0;

	array <double,AC> totalIntACCol = {};
	double totalIntCol = 0.0;

	array <double,AC> droppedAC = {};
	double totalDropped = 0.0;


	for (int i = 0; i < Nodes; i++){
		for (int j = 0; j < AC; j++){
			//#1 Throughput
			totalThroughput += stas[i].overallACThroughput.at(j);
			totalACthroughput.at(j) += stas[i].overallACThroughput.at(j);

			totalSxTx += (stas[i].successfulTx.at(j));
			overallSxTx.at(j) += (stas[i].successfulTx.at(j));
			
			totalTx += (stas[i].transmissions.at(j));
			overallTx.at(j) += (stas[i].transmissions.at(j));

			totalRetransmissions += (stas[i].totalACRet.at(j));
			totalACRet.at(j) += (stas[i].totalACRet.at(j));

			totalCol += (stas[i].totalACCollisions.at(j));
			totalACCol.at(j) += (stas[i].totalACCollisions.at(j));

			totalIntCol += (stas[i].totalInternalACCol.at(j));
			totalIntACCol.at(j) += (stas[i].totalInternalACCol.at(j));

			totalDropped += (stas[i].droppedAC.at(j));
			droppedAC.at(j) += (stas[i].droppedAC.at(j));
		}
	}

	ofstream file;
	file.open("Results/output.txt", ios::app);
	file << "#1. Nodes 2. totalThroughput 3. totalBEThroughput 4. totalBKThroughput 5. totalVIThroughput " << endl;
	file << "#6. totalVOThroughput 7. totalCollisions 8. totalBECollisions 9. totalBKCollisions" << endl;
	file << "#10. totalVICollisions 11. TotalVOCollisions 12. totalInternalCollisions" << endl;
	file << "#13. totalBEIntCol 14. totalBKIntCol 15. totalVIIntCol 16. totalVOIntCol" << endl;
	
	file << Nodes << " " << totalThroughput << " ";
	//Printing AC related metrics
	//3 - 6
	for (int i = 0; i < AC; i++){
		file << totalACthroughput.at(i) << " ";
	}

	//7-11
	file << totalCol << " ";
	for (int i = 0; i < AC; i++){
		file << totalACCol.at(i) << " ";
	}

	//12-16
	file << totalIntCol << " ";
	for (int i = 0; i < AC; i++){
		file << totalIntACCol.at(i) << " ";
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
	for(int i = 0; i < AC; i++)
	{
		cout << "\tAC " << i << ": " << overallTx.at(i) << ". Total Throughput for AC (Mbps): " 
			<< totalACthroughput.at(i) << endl;;

	}


	cout << "\n2. Total Collisions: " << totalCol << ". Collisions / Transmitted ratio: " 
		<< totalCol / totalTx << endl;
	cout << "2.1 Total Internal Collisions: " << totalIntCol << ". Internal Collisions + Collisions: "
		<< totalIntCol + totalCol << endl;

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
		<< totalDropped / totalSxTx << endl;

	for(int i = 0; i < AC; i++)
	{
		cout << "\tAC " << i << ": " << droppedAC.at(i) << ". Dropped AC / SxSent ratio: "
			<< droppedAC.at(i) / totalSxTx << endl;
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
	int hysteresis;
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
				cout << "(0)./XXXX (1)SimTime (2)NumNodes (3)PacketLength (4)Bandwidth (5)Batch (6)Stickiness (7)hysteresis (8)fairShare (9)channelErrors (10)slotDrift (11)percentageOfEDCA (12)maxAggregation (13)simSeed" << endl << endl;;
				cout << "0) ./XXX: Name of executable file" << endl;
				cout << "1) SimTime: simulation time in seconds" << endl;
				cout << "2) NumNodes: number of contenders" << endl;
				cout << "3) PacketLength: length of the packet in bytes" << endl;
				cout << "4) Bandwidth: number of bits per second generated by the source. With 802.11n and EDCA, 10e6 < is considered an unsaturated environment." << endl;
				cout << "5) Batch: how many packets are put in the contenders queue. Used to simulate burst traffic. Usually set to 1" << endl;
				cout << "6) Stickiness: activates CSMA/ECA. Nodes pick a deterministic backoff (0=off, 1=on)" << endl;
				cout << "7) Hysteresis: nodes do not reset their backoff stage after successful transmissions (0=off, 1=on)" << endl;
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
			cout << "./XXXX SimTime [10] NumNodes [2] PacketLength [1024] Bandwidth [65e6] Batch [1] Stickiness [0] hysteresis [0] fairShare [0] channelErrors [0] slotDrift [0] percentageOfEDCA [1] maxAggregation [0] simSeed [0]" << endl;
			MaxSimIter = 1;
			SimTime = 10;
			NumNodes = 30;
			PacketLength = 1024;
			Bandwidth = 65e6;
			Batch = 1; // =1
			Stickiness = 0; // 0 = EDCA, up to 2.
			hysteresis = 0; //keep the current BO stage, until queue's empty
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
		Stickiness = atoi(argv[6]); // 0 = EDCA, up to 2.
		hysteresis = atoi(argv[7]); //keep the current BO stage, until queue's empty
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
		if(hysteresis > 0)
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

	test.Setup(MaxSimIter,NumNodes,PacketLength,Bandwidth,Batch,Stickiness, hysteresis, fairShare, channelErrors, slotDrift, percentageEDCA, maxAggregation, simSeed);
	
	test.Run();


	return(0);
};
