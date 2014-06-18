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

void SlottedCSMA :: Setup(int Sim_Id, int NumNodes, int PacketLength, double Bandwidth, int Batch, int Stickiness, int hysteresis, int fairShare, float channelErrors, float slotDrift, float percentageDCF, int maxAggregation, int simSeed)
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
	cut = NumNodes * percentageDCF;
	decimalCut = modf(cut, &intCut);
	
	if(decimalCut > 0.5)
	{
		intCut++;	
	}
		
	/*cout << "Cut: " << (int)intCut << endl;
	cout << "Nodes: " << NumNodes << endl;
	cout << "Percentage: " << percentageDCF << endl;*/
	
	
	for(int n=0;n<NumNodes;n++)
	{
		// Node
		stas[n].node_id = n;
		stas[n].K = 1000;
		stas[n].system_stickiness = Stickiness;
		stas[n].station_stickiness = 0;
		stas[n].hysteresis = hysteresis;
		stas[n].fairShare = fairShare;
		stas[n].driftProbability = slotDrift;
		stas[n].cut = intCut;     
		stas[n].maxAggregation = maxAggregation;


		// Traffic Source
		sources[n].bandwidth = Bandwidth;
		sources[n].L = PacketLength;
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
	printf("--------------- SlottedCSMA ---------------\n");
};

void SlottedCSMA :: Stop()
{
	double p_res = 0;
	double delay_res = 0;
	
	double overall_successful_tx = 0;
	double overall_collisions = 0;
	double overall_empty = 0;
	double total_slots = 0;
	double overall_successful_tx_slots = 0;
	double driftedSlots = 0;
	double tx_slots = 0;
	double overall_throughput = 0;
	
	

	double avg_tau = 0;
	double std_tau = 0;
	
	double stas_throughput [Nodes];
	double stas_throughputDCF [Nodes];
	double stas_throughputECA [Nodes];
	double accumThroughputDCF = 0;
	double accumThroughputECA = 0;
	
	int DCFStas = 0;
	int ECAStas = 0;
	
	double fairness_index = 0;
	double systemTXDelay = 0.0;
	double systemAvgBlockingProbability = 0;
	
	double avgBackoffStage = 0;
	double accumaltedDroppedPackets = 0;
	double avgFinalQSize = 0;
	double QEmpties = 0; //number of ocassions the Q empties in every simulation

	
	for(int n=0;n<Nodes;n++)
	{
	    avg_tau += ((float)stas[n].total_transmissions / (float)stas[n].observed_slots);
	    driftedSlots += stas[n].driftedSlots;
	    tx_slots += stas[n].total_transmissions;
	    overall_successful_tx+=stas[n].successful_transmissions;
	    avgBackoffStage += stas[n].finalBackoffStage;
	}
	overall_collisions = channel.collision_slots;
	overall_empty = channel.empty_slots;
	total_slots = channel.total_slots;
	overall_successful_tx_slots = channel.succesful_slots;
	driftedSlots /= tx_slots;

	p_res /= Nodes;
	delay_res /= Nodes;
	
	avg_tau /= Nodes;
	
	//temporal statistics
	avgBackoffStage /= Nodes;
	cout << endl;
	
	//Computing the standard deviation of each of the station's tau
	//Also capturing each station's throughput to build the Jain's index
	//And the delay of each station to derive a system average txDalay delay
	
	//Creating the file for recording station's individual statistics
	ofstream staStatistics;
	staStatistics.open("Results/multiStation.txt", ios::app);
	
	for(int i=0; i<Nodes; i++)
	{
	    std_tau += pow(avg_tau - ((float)stas[i].total_transmissions / (float)stas[i].observed_slots),2);
	    stas_throughput[i] = stas[i].throughput;
	    systemTXDelay += stas[i].staDelay;
	    
	    //cout << i << " " << stas[i].staDelay << endl;
	    //if(stas[i].qEmpty > 1)cout << "Station: " << i << " emptied the queue " << stas[i].qEmpty << " times" << endl;
	    
	    QEmpties += stas[i].qEmpty;
	    
	    //Separating the collection of throughput of DCF and ECA stations
	    if(stas[i].DCF > 0)
	    {
	    	stas_throughputDCF[i] = stas[i].throughput;
	    	stas_throughputECA[i] = 0;
	    	DCFStas++;
	    }else
	    {
	    	stas_throughputECA[i] = stas[i].throughput;
	    	stas_throughputDCF[i] = 0;
	    	ECAStas++;
	    }
	    accumThroughputDCF += stas_throughputDCF[i];
	    accumThroughputECA += stas_throughputECA[i];
	    accumaltedDroppedPackets += stas[i].droppedPackets;
	    avgFinalQSize += stas[i].qSize;
	    
	    //Below is the if statement for checking that the number of incoming packets is equal to the transmitted + blocked + the ones in the queue
	    if(stas[i].incoming_packets == stas[i].successful_transmissions + stas[i].blocked_packets + stas[i].qSize + stas[i].droppedPackets)
	    {	
	    	cout << "Station " << i << ": is alright" << endl;
	    }else
	    {
	    	cout << "Station " << i << ": differs in " << stas[i].incoming_packets - (stas[i].successful_transmissions + stas[i].blocked_packets + stas[i].qSize + stas[i].droppedPackets) << endl;
	    }
	    
	    //Gathering the average blocking probability
	    systemAvgBlockingProbability += stas[i].blockingProbability;
	    
	    //-----------------------------------------------
	    //Gathering statistics from nodes for staMultiSim
	    //-----------------------------------------------
	    // 1.throughput 2.collisions 3.TAU 4.Delay 5.QEmpty 6.QSize 7.avgBackoffStage 8.DroppedPackets
		staStatistics << i << " " << stas[i].throughput << " " << stas[i].collisions / stas[i].total_transmissions << " " << stas[i].total_transmissions / stas[i].observed_slots << " " << stas[i].staDelay << " " << stas[i].qEmpty << " " << stas[i].qSize << " " << stas[i].finalBackoffStage << " " << stas[i].droppedPackets << endl;
	}
	
	std_tau = pow((1.0/Nodes) * (float)std_tau, 0.5);
	
	systemTXDelay /= Nodes;
	
	systemAvgBlockingProbability /= Nodes;
	avgFinalQSize /= Nodes;
	
	
	double fair_numerator, fair_denominator;
	
	fair_numerator = 0;
	fair_denominator = 0;
	
	//computing the fairness_index
	for(int k = 0; k < Nodes; k++)
	{
	    fair_numerator += stas_throughput[k];
	    fair_denominator += pow(stas_throughput[k],2);        
	}
	
	fairness_index = (pow(fair_numerator,2)) / (Nodes*fair_denominator);
	
	//802.11n version
	overall_throughput = (channel.totalBitsSent)/SimTime();

	ofstream statistics;
	statistics.open("Results/multiSim.txt", ios::app);
	statistics << Nodes << " " << overall_throughput << " " << overall_collisions / total_slots  << " " << fairness_index  << " " << Bandwidth_ << " " << systemTXDelay << " " << avgBackoffStage << " "; 
	if(DCFStas > 0){ 
	    statistics << accumThroughputDCF/DCFStas;
	}else
	{
	    statistics << accumThroughputDCF;
	}statistics << " ";
	if(ECAStas > 0){ 
	    statistics << accumThroughputECA/ECAStas;
	}else
	{
	    statistics << accumThroughputECA;
	}statistics << " " << fair_numerator << " ";
	
	statistics << systemAvgBlockingProbability << " " << accumaltedDroppedPackets/Nodes << " " << overall_successful_tx_slots << " " << overall_collisions << " " << overall_empty << " " << total_slots << " " << avg_tau  << " " << avgFinalQSize << " " << QEmpties << endl;
	
	cout << endl << endl;
	
	//--------------------------------------------------------------//
	//---------Presentation when simulation ends--------------------//
	//--------------------------------------------------------------//
	
	cout << "--- Overall Statistics ---" << endl;
	cout << "Average TAU = " << avg_tau << endl;
	cout << "Standard Deviation = " << (double)std_tau << endl;
	cout << "Overall Throughput = " << overall_throughput << endl;
	
	//They differ just a little
	if((fair_numerator != (accumThroughputDCF + accumThroughputECA)) && (fair_numerator - (accumThroughputDCF+accumThroughputECA) > 1))
	{
		cout << "Error gathering the throughput of each station" << endl;
		cout << "Total: " << fair_numerator << " DCF: " << accumThroughputDCF << ", ECA: " << accumThroughputECA << ", diferring in: " << fair_numerator - (accumThroughputDCF+accumThroughputECA) << endl;
	}
	
	cout << "Jain's Fairness Index = " << fairness_index << endl;
	cout << "Overall average system TX delay = " << systemTXDelay << endl;
	cout << "Percentage of drifted slots = " << driftedSlots*100 << "%" << endl << endl;
	
	
	cout << "***Debugg***" << endl;
	cout << "Average backoff stage [0-5]: " << avgBackoffStage << endl;
	cout << "Average number of dropped packets: " << accumaltedDroppedPackets/Nodes << endl;
	cout << "Average blocking proability: " << systemAvgBlockingProbability << endl;
	cout << "Number of times each MAC queue emptied: " << QEmpties << endl;
	cout << "Slot drift probability: " << drift*100 << "%" << endl;
	cout << "Sx Slots: " << overall_successful_tx_slots << endl;
	cout << "Collision Slots: " << overall_collisions << endl;
	cout << "Empty Slots: " << overall_empty << endl;
	cout << "Total Slots: " << total_slots << endl;
	if(total_slots != (overall_successful_tx_slots+overall_collisions+overall_empty))
	{
	    cout << "They differ by: " << fabs(total_slots - (overall_successful_tx_slots+overall_collisions+overall_empty)) << endl;    
	}else
	{
	    cout << "Total Slots = Sucessful + Collision + Empty" << endl;
	}
	
	cout << "Total bits sent: " << channel.totalBitsSent << " if divided by " << SimTime() << "seconds of simulation, equals = " << (channel.totalBitsSent)/SimTime() << endl << endl;

	cout << "---Debugging the mixed scenario---" << endl;
	cout << "There are: " << DCFStas << " stations with DCF and: " << ECAStas << " with CSMA/ECA." << endl;
	if(Nodes != (DCFStas + ECAStas)) cout << "Miscount of stations" << endl;
	
	//Avoiding divisions by zero on the cout
	if(ECAStas == 0) ECAStas = 1;
	if(DCFStas == 0) DCFStas = 1;
	cout << "The average throughput of DCF stations is: " << accumThroughputDCF/DCFStas << "bps" << endl;
	cout << "The average throughput of Full CSMA/ECA staions is: " << accumThroughputECA/ECAStas << "bps" << endl;
	if((accumThroughputECA == 0) || (accumThroughputDCF == 0))
	{
		cout << "Some stations received no throughput, so the CSMA/ECA / CSMA/CA cannot be computed" << endl;
	}else
	{
		cout << "CSMA/ECA / CSMA/CA ratio: " << (accumThroughputECA/ECAStas)/(accumThroughputDCF/DCFStas) << endl;
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
	float percentageDCF;
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
				cout << "(0)./XXXX (1)SimTime (2)NumNodes (3)PacketLength (4)Bandwidth (5)Batch (6)Stickiness (7)hysteresis (8)fairShare (9)channelErrors (10)slotDrift (11)percentageOfDCF (12)maxAggregation (13)simSeed" << endl << endl;;
				cout << "0) ./XXX: Name of executable file" << endl;
				cout << "1) SimTime: simulation time in seconds" << endl;
				cout << "2) NumNodes: number of contenders" << endl;
				cout << "3) PacketLength: length of the packet in bytes" << endl;
				cout << "4) Bandwidth: number of bits per second generated by the source. With 802.11n and DCF, 10e6 < is considered an unsaturated environment." << endl;
				cout << "5) Batch: how many packets are put in the contenders queue. Used to simulate burst traffic. Usually set to 1" << endl;
				cout << "6) Stickiness: activates CSMA/ECA. Nodes pick a deterministic backoff (0=off, 1=on)" << endl;
				cout << "7) Hysteresis: nodes do not reset their backoff stage after successful transmissions (0=off, 1=on)" << endl;
				cout << "8) FairShare: nodes at backoff stage k, attempt the transmission of 2^k packets (0=off, 1=on)" << endl;
				cout << "9) ChannelErrors: channel errors probability [0,1]" << endl;
				cout << "10) SlotDrift: probability of miscounting passing empty slots [0,1]" << endl;
				cout << "11) PercetageDCF: percentage of nodes running DCF. Used to simulate CSMA/ECA and CSMA/CA mixed scenarios [0,1]" << endl;
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
			cout << "./XXXX SimTime [10] NumNodes [10] PacketLength [1024] Bandwidth [65e6] Batch [1] Stickiness [0] hysteresis [0] fairShare [0] channelErrors [0] slotDrift [0] percentageOfDCF [1] maxAggregation [0] simSeed [0]" << endl;
			MaxSimIter = 1;
			SimTime = 10;
			NumNodes = 10;
			PacketLength = 1024;
			Bandwidth = 65e6;
			Batch = 1; // =1
			Stickiness = 0; // 0 = DCF, up to 2.
			hysteresis = 0; //keep the current BO stage, until queue's empty
			fairShare = 0; //0 = DCF, 1 = CSMA-ECA
			channelErrors = 0; // float 0-1
			slotDrift = 0; // // float 0-1
			percentageDCF = 1; // // float 0-1
			maxAggregation = 0;
			simSeed = 0; //Simulation seed
		}
	}else
	{
		MaxSimIter = 1;
		SimTime = atof(argv[1]);
		NumNodes = atoi(argv[2]);
		PacketLength = atoi(argv[3]);
		Bandwidth = atof(argv[4]);
		Batch = atoi(argv[5]); // =1
		Stickiness = atoi(argv[6]); // 0 = DCF, up to 2.
		hysteresis = atoi(argv[7]); //keep the current BO stage, until queue's empty
		fairShare = atoi(argv[8]); //0 = DCF, 1 = CSMA-ECA
		channelErrors = atof(argv[9]); // float 0-1
		slotDrift = atof(argv[10]); // // float 0-1
		percentageDCF = atof(argv[11]); // // float 0-1
		maxAggregation = atoi(argv[12]); //0 = no, 1 = yes
		simSeed = atof(argv[13]); //Simulation seed
	}

	printf("####################### Simulation (Seed: %d) #######################\n",simSeed);
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
	
	if(percentageDCF > 0) cout << "####################### Mixed setup " << percentageDCF*100 << "% DCF #######################" << endl;
		
	SlottedCSMA test;

	//test.Seed=(long int)6*rand();
	
	test.Seed = simSeed;
		
	test.StopTime(SimTime);

	test.Setup(MaxSimIter,NumNodes,PacketLength,Bandwidth,Batch,Stickiness, hysteresis, fairShare, channelErrors, slotDrift, percentageDCF, maxAggregation, simSeed);
	
	test.Run();


	return(0);
};
