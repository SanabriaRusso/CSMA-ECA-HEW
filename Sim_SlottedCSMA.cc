#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <iostream>
#include <fstream>
#include <assert.h>

#include "./COST/cost.h"

#include <deque>

#include "Channel.h"
#include "STA.h"
// #include "BatchPoissonSource.h"
#include "BatchPoissonSource_enhanced.h"
#include "stats/stats.h"

#define AC 4

using namespace std;

component SlottedCSMA : public CostSimEng
{
	public:
		void Setup(int Sim_Id, int NumNodes, int PacketLength, double Bandwidth, int Batch, int Stickiness, int ECA, int fairShare, float channelErrors, float slotDrift,float percentageDCF, int maxAggregation, int howManyACs, int simSeed);
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
		double percentageEDCA_;
		int Batch_;
		float drift;
		double intCut, decimalCut, cut; 
		

};

void SlottedCSMA :: Setup(int Sim_Id, int NumNodes, int PacketLength, double Bandwidth, int Batch, int Stickiness, int ECA, int fairShare, float channelErrors, float slotDrift, float percentageEDCA, int maxAggregation, int howManyACs, int simSeed)
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
	if(percentageEDCA == 0 || percentageEDCA == 1)
	{
		intCut = percentageEDCA;
	}else
	{
		cut = NumNodes * percentageEDCA;
		decimalCut = modf(cut, &intCut);
		
		if(decimalCut > 0.5)
		{
			intCut++;	
		}
	}
	
	for(int n=0;n<NumNodes;n++)
	{
		// Node
		stas[n].node_id = n;
		stas[n].nodesInSim = Nodes;
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
		sources[n].categories = howManyACs;
		sources[n].packetsGenerated = 0;
		for(int i = 0; i < AC; i++) sources[n].packetsInAC.at(i) = 0;		

		// The percentage of generated packets destined to a specific AC
		// This distribution of the load produces predictable plots
		// THIS IS ABOUT TO BE CHANGED BY G.711 and VI codecs traffic sources simulators
		sources[n].BKShare = 100; // 40%
		sources[n].BEShare = 60;  // 30%
		sources[n].VIShare = 30;  // 15%
		sources[n].VOShare = 15;  // 15%
		sources[n].MaxBatch = Batch;
	}

	// Still traffic source
	// Implementing a faster source
	// If bandwidth is below 10e6, half the stations will have said bandwidth,
	// while the other half will have ten times that value.
	// This creates a mixed saturation/non-saturation environment which is used to test the schedule reset.
	// if(Bandwidth < 10e6)
	// {
	// 	for(int i = 0; i < NumNodes/4; i++)
	// 	{
	// 		sources[i].packet_rate = Bandwidth/(PacketLength * 8);		
	// 		stas[i].saturated = 0;
	// 	}
	// 	for(int i = NumNodes/4; i < NumNodes/2; i++)
	// 	{
	// 		sources[i].packet_rate = (Bandwidth*10)/(PacketLength * 8);			
	// 		stas[i].saturated = 1;
	// 	}	
	// 	for(int i = NumNodes/2; i < (3*NumNodes/4); i++)
	// 	{
	// 		sources[i].packet_rate = Bandwidth/(PacketLength * 8);		
	// 		stas[i].saturated = 0;
	// 	}
	// 	for(int i = (3*NumNodes/4); i < NumNodes; i++)
	// 	{
	// 		sources[i].packet_rate = (Bandwidth*10)/(PacketLength * 8);			
	// 		stas[i].saturated = 1;
	// 	}	
	// }else
	// {
		for(int i = 0; i < NumNodes; i++)
		{
			sources[i].packet_rate = Bandwidth/(PacketLength * 8);
			stas[i].saturated = 1;
			if(Bandwidth < 10e6) stas[i].saturated = 0;
		}
	// }
	//*DEBUG
	// for(int i = 0; i < NumNodes; i++)
	// {
	// 	cout << "Node-" << i << " packet rate: " << sources[i].packet_rate << endl;
	// }
	//End traffic source
	
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
	percentageEDCA_ = percentageEDCA;
		

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
	array <double,AC> totalACthroughputEDCA = {};
	array <double,AC> totalACthroughputECA = {};
	double totalThroughput = 0.0;
	double totalThroughputEDCA = 0.0;
	double totalThroughputECA = 0.0;
	
	array <double,AC> overallSxTx = {};
	double totalSentPackets = 0.0;

	array <double,AC> overallTx = {};
	double totalTx = 0.0;
	array <double,AC> totalACRet = {};
	double totalRetransmissions = 0.0;

	array <double,AC> totalACCol = {};
	array <double,AC> totalACColEDCA = {};
	array <double,AC> totalACColECA = {};
	double totalCol = channel.collision_slots;
	double sumOfCol = 0.0;
	double lastCollision = 0.0;

	array <double,AC> totalIntACCol = {};
	double totalIntCol = 0.0;

	array <double,AC> droppedAC = {};
	double totalDropped = 0.0;

	double totalIncommingPackets = 0;
	double totalErasedPackets = 0;
	double totalRemainingPackets = 0;

	array <double,AC> fairnessAC = {};
	array <double,AC> fairnessACNum = {};
	array <double,AC> fairnessACDenom = {};
	double overallFairnessNum = 0.0;
	double overallFairnessDenom = 0.0;

	array <double,AC> avgTimeBetweenACSxTx = {};
	array <double,AC> avgQueueingDelay = {};
	array <double,AC> avgTimeBetweenACSxTxEDCA = {};
	array <double,AC> avgTimeBetweenACSxTxECA = {};

	array <double,AC> qEmpties = {};

	double totalSource = 0.0;
	array <double,AC> sourceForAC = {};
    array <double,AC> avgBackoffStageECA = {};
    array <double,AC> avgBackoffStageDCF = {};



	for (int i = 0; i < Nodes; i++){
		for (int j = 0; j < AC; j++){
			if(stas[i].overallACThroughput.at(j) > 0)
			{
				totalThroughput += stas[i].overallACThroughput.at(j);
				totalACthroughput.at(j) += stas[i].overallACThroughput.at(j);
				if(stas[i].ECA == 0)
				{
					totalThroughputEDCA += stas[i].overallACThroughput.at(j);
					totalACthroughputEDCA.at(j) += stas[i].overallACThroughput.at(j);
				}else{
					totalThroughputECA += stas[i].overallACThroughput.at(j);
					totalACthroughputECA.at(j) += stas[i].overallACThroughput.at(j);
				}
			}

			if(stas[i].packetsSent.at(j) > 0)
			{
				totalSentPackets += (stas[i].packetsSent.at(j));
				overallSxTx.at(j) += (stas[i].packetsSent.at(j));
			}
			
			if(stas[i].transmissions.at(j) > 0)
			{
				totalTx += (stas[i].transmissions.at(j));
				overallTx.at(j) += (stas[i].transmissions.at(j));
			}

			if(stas[i].totalACRet.at(j) > 0)
			{
				totalRetransmissions += (stas[i].totalACRet.at(j));
				totalACRet.at(j) += (stas[i].totalACRet.at(j));
			}

			// if(stas[i].totalACCollisions.at(j) > 0)
			// {
				sumOfCol += (stas[i].totalACCollisions.at(j));
				totalACCol.at(j) += ( stas[i].totalACCollisions.at(j) / stas[i].transmissions.at(j) );
				if(stas[i].ECA == 0)
				{
					totalACColEDCA.at(j) += ( stas[i].totalACCollisions.at(j) / stas[i].transmissions.at(j) );
				}else{
					totalACColECA.at(j) += ( stas[i].totalACCollisions.at(j) / stas[i].transmissions.at(j) );
				}
			// }

			if(stas[i].totalInternalACCol.at(j) > 0)
			{
				totalIntCol += (stas[i].totalInternalACCol.at(j));
				totalIntACCol.at(j) += (stas[i].totalInternalACCol.at(j));
			}

			if(stas[i].droppedAC.at(j) > 0)
			{
				totalDropped += (stas[i].droppedAC.at(j));
				droppedAC.at(j) += (stas[i].droppedAC.at(j));
			}

			if(stas[i].overallACThroughput.at(j) > 0)
			{
				fairnessACNum.at(j) += (stas[i].overallACThroughput.at(j));
				overallFairnessNum += (stas[i].overallACThroughput.at(j));
				fairnessACDenom.at(j) += (double)pow(stas[i].overallACThroughput.at(j), 2);
				overallFairnessDenom += (double)pow(stas[i].overallACThroughput.at(j), 2);
			}

			if(stas[i].sxTx.at(j) > 0)
			{
				avgTimeBetweenACSxTx.at(j) += (stas[i].accumTimeBetweenSxTx.at(j) / stas[i].sxTx.at(j));
				avgQueueingDelay.at(j) += stas[i].accumQueueingDelay.at(j);
				if(stas[i].ECA == 0)
				{
					avgTimeBetweenACSxTxEDCA.at(j) += (stas[i].accumTimeBetweenSxTx.at(j) / stas[i].sxTx.at(j));
				}else
				{
					avgTimeBetweenACSxTxECA.at(j) += (stas[i].accumTimeBetweenSxTx.at(j) / stas[i].sxTx.at(j));
				}
			}
			
			qEmpties.at(j) += (stas[i].queueEmpties.at(j));

			totalSource += (sources[i].packetsInAC.at(j));
			sourceForAC.at(j) += (sources[i].packetsInAC.at(j));

			if(lastCollision < stas[i].lastCollision.at(j)) lastCollision = stas[i].lastCollision.at(j);
            
            if(stas[i].ECA == 0){
                avgBackoffStageDCF.at(j) += stas[i].backoffStagesFinal.at(j);
            }else{
                avgBackoffStageECA.at(j) += stas[i].backoffStagesFinal.at(j);
            }

		}
		totalIncommingPackets += (stas[i].incommingPackets);
		totalErasedPackets += (stas[i].erased);
		totalRemainingPackets += (stas[i].remaining);

	}

	ofstream file;
	file.open("Results/output.txt", ios::app);
	file << "#1. Nodes 					2. totalThroughput 			3. totalBKThroughput 		4. totalBEThroughput "<< endl;
	file << "#5. totalVIThroughput 		6. totalVOThroughput 		7. totalCollisionsSlots 	8. fractionBKCollisions "<< endl;
	file << "#9. fractionBECollisions	10. fractionVICollisions 	11. fractionVOCollisions 	12. totalInternalCollisions" << endl;
	file << "#13. totalBKIntCol 		14. totalBEIntCol 			15. totalVIIntCol 	 		16. totalVOIntCol" << endl;
	file << "#17. overallFairness 		18. BKFairness				19. BEFairness				20. VIFairness"	<< endl;
	file << "#21. VOFairness			22. avgTimeBtSxTxBK			23. avgTimeBtSxTxBE			24. avgTimeBtSxTxVI" << endl;
	file << "#25. avgTimeBtSxTxVO		26. qEmptyBK				27. qEmptyBE				28. qEmptyVI" << endl;
	file << "#29. qEmptyVO				30. totalDropped			31. droppedBK				32. droppedBE" << endl;
	file << "#33. droppedVI				34. droppedVO				35. channelErrors           36. Stickiness" << endl;
	file << "#37. totalThroughputUnsat	38. totalThroughputSat		39. totalThroughputEDCA		40. EDCABKthroughput" << endl;
	file << "#41. EDCABEthroughput		42. EDCAVIthroughput		43. EDCAVOthroughput		44.totalThroughputECA" << endl;
	file << "#45. ECABKthroughput 		46. ECABEthroughput 		47. ECAVIthroughput 		48. ECAVOthroughput" << endl;
	file << "#49 avgTimeBtSxTxBK-EDCA	50. avgTimeBtSxTxBE-EDCA 	51. avgTimeBtSxTxVI-EDCA	52. avgTimeBtSxTxVO-EDCA" << endl;
	file << "#53 avgTimeBtSxTxBK-ECA 	54. avgTimeBtSxTxBE-ECA 	55. avgTimeBtSxTxVI-ECA 	56. avgTimeBtSxTxVO-ECA" << endl;
	file << "#57. fractBKCollisionsEDCA	58. fractBECollisionsEDCA 	59. fractVICollisionsEDCA 	60. fractVOCollisionsEDCA" << endl;
	file << "#61. fractBKCollisionsECA 	62. fractBECollisionsECA 	63. fractVICollisionsECA 	64. fractVOCollisionsECA" << endl;
	file << "#65. lastCollision			66. avgQueueingDelayBK		67. avgQueueingDelayBE		68.avgQueueingDelayVI" << endl;
	file << "#69. avgQueueingDelayVO    70. avgBackoffStageECABK    71. avgBackoffStageDCFBK	72. percentageEDCA_" << endl;

	file << Nodes << " " << totalThroughput << " ";
	//Printing AC related metrics
	//3 - 6
	for (int i = 0; i < AC; i++){
		file << totalACthroughput.at(i) << " ";
	}

	//7-11
	file << totalCol << " ";
	for (int i = 0; i < AC; i++){
		if(totalACCol.at(i) > 0)
		{
			file << (double)(totalACCol.at(i)/Nodes) << " ";
		}else
		{
			file << "0 ";
		}
	}

	//12-16
	file << totalIntCol << " ";
	for (int i = 0; i < AC; i++){
		file << totalIntACCol.at(i) << " ";
	}

	//17-21
	file << (double) ( (pow(overallFairnessNum,2)) / (Nodes * (overallFairnessDenom)) ) << " ";
	for (int i = 0; i < AC; i++){
		if(fairnessACDenom.at(i) > 0) 
		{
			fairnessAC.at(i) = (double) ( (pow(fairnessACNum.at(i),2)) / (Nodes * (fairnessACDenom.at(i))) );
		}else
		{
			fairnessAC.at(i) = 0.0;
		}
		
		file << fairnessAC.at(i) << " ";
	}

	//22-25
	for(int i = 0; i < AC; i++){
		if(avgTimeBetweenACSxTx.at(i) != 0)
		{
			file << (double)avgTimeBetweenACSxTx.at(i)/Nodes << " ";
		}else
		{
			file << "0 ";
			avgTimeBetweenACSxTx.at(i) = 0.0;
		}
	}

	//26-29
	for(int i = 0; i < AC; i++){
		if(qEmpties.at(i) != 0)
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

	//35
	file << channel.error << " ";

	//36
	file << stas[0].system_stickiness << " ";

	//37-38
	double totalThroughputUnsat = 0.0;
	double totalThroughputSat = 0.0;
	for(int i = 0; i < Nodes; i++)
	{
		if(stas[i].saturated == 0)
		{		
			totalThroughputUnsat += stas[i].overallThroughput;
		}else
		{
			totalThroughputSat += stas[i].overallThroughput;		
		}
	}
	file << totalThroughputUnsat << " " << totalThroughputSat << " ";

	//39-43
	file << totalThroughputEDCA << " ";

	for (int i = 0; i < AC; i++){
		file << totalACthroughputEDCA.at(i) << " ";
	}

	//44-48
	file << totalThroughputECA << " ";
	for (int i = 0; i < AC; i++){
		file << totalACthroughputECA.at(i) << " ";
	}

	//49-52
	int EDCAnodes = 0;
	int ECAnodes = 0;
	for(int n = 0; n < Nodes; n++)
	{
		if(stas[n].ECA == 0)
		{
			EDCAnodes++;
		}else
		{
			ECAnodes++;
		}
	}
	for(int i = 0; i < AC; i++){
		if(avgTimeBetweenACSxTxEDCA.at(i) != 0)
		{
			file << (double)avgTimeBetweenACSxTxEDCA.at(i)/EDCAnodes << " ";
		}else
		{
			file << "0 ";
			avgTimeBetweenACSxTxEDCA.at(i) = 0.0;
		}
	}

	//53-56
	for(int i = 0; i < AC; i++){
		if(avgTimeBetweenACSxTxECA.at(i) != 0)
		{
			file << (double)avgTimeBetweenACSxTxECA.at(i)/ECAnodes << " ";
		}else
		{
			file << "0 ";
			avgTimeBetweenACSxTxECA.at(i) = 0.0;
		}
	}

	//57-60
	for (int i = 0; i < AC; i++){
		if(totalACColEDCA.at(i) > 0)
		{
			file << (double)(totalACColEDCA.at(i)/EDCAnodes) << " ";
		}else
		{
			file << "0 ";
		}
	}

	//61-64
	for (int i = 0; i < AC; i++){
		if(totalACColECA.at(i) != 0)
		{
			file << (double)(totalACColECA.at(i)/ECAnodes) << " ";
		}else{
			file << "0 ";
		}
	}

	//65
	file << lastCollision << " ";

	//66-69
	for(int i = 0; i < AC; i++){
		if(avgQueueingDelay.at(i) > 0){
			file << avgQueueingDelay.at(i)/Nodes << " ";
		}else{
			file << "0 ";
		}
	}
    
    //70-71
    if(avgBackoffStageECA.at(0) > 0){
        file << avgBackoffStageECA.at(0)/(Nodes*percentageEDCA_) << " ";
    }else{
        file << "0 ";
    }
    
    if(avgBackoffStageDCF.at(0) > 0){
        file << avgBackoffStageDCF.at(0)/(Nodes*percentageEDCA_) << " ";
    }else{
        file << "0 ";
    }

    //72
    file << percentageEDCA_ << " ";

	file << endl;


	//--------------------------------------------------------------//
	//---------Presentation when simulation ends--------------------//
	//--------------------------------------------------------------//

	cout << endl;
	cout << "---------------------------"<< endl;
	cout << "--- Overall Statistics ----" << endl;
	cout << "---------------------------"<< endl;

	double count = channel.succesful_slots  + channel.collision_slots + channel.error_slots + channel.empty_slots;
	if(count == channel.total_slots)
	{
	cout << "0. Channel Statistics: total slots " << channel.total_slots << ", sx slots " << channel.succesful_slots <<
		", colision slots " << channel.collision_slots << ", channel error slots " << channel.error_slots << ", empty slots "
		<< channel.empty_slots << endl << endl;
	}

	cout << "1. Total transmissions: " << totalTx << ". Total Throughput (Mbps): " << totalThroughput << endl;
	cout << "1.1 Packet generation rate: " << Bandwidth_ << endl;
	for(int i = 0; i < AC; i++)
	{
		cout << "\tAC " << i << ": " << overallTx.at(i) << ". Total Throughput for AC: " 
			<< totalACthroughput.at(i) << endl;;

	}

	cout << "\n2. Total Collisions: " << totalCol << endl;
	cout << "2.1 Total Internal Collisions: " << totalIntCol << endl;
	cout << "2.2 Summing collision metrics from all stations" << endl;
	for(int i = 0; i < AC; i++)
	{
		cout << "\tAC " << i << ": " << totalACCol.at(i) / Nodes << ". Collisions AC / Total Transmitted." << endl;
		cout << "\tInternal Collisions: " << totalIntACCol.at(i) << endl << endl;
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
	totalIncommingPackets / (totalErasedPackets + totalRemainingPackets) <<  endl;
	
	// cout <<"\t***DEBUG. totalIncommingPackets: " << totalIncommingPackets << ", totalErasedPackets: " << totalErasedPackets <<
	// 	", totalRemainingPackets: " << totalRemainingPackets << endl;


	cout << "\n6. Overall Fairness: " << (double)((pow(overallFairnessNum,2))/(Nodes*(overallFairnessDenom))) << endl;

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

	cout << "\n9. Packets generated: " << totalSource << endl;

	for(int i = 0; i < AC; i++)
	{
		cout << "\tAC " <<  i << ": " << sourceForAC.at(i) << endl;
	}

	// cout << "***DEBUG. Generated / Received by STA's MAC = " << totalSource/totalIncommingPackets << endl;

	cout << "\n10. Proportion of EDCA nodes: " << percentageEDCA_*100 << "%" << endl;
	
	// *DEBUG
	// cout << "Global cut: " << intCut << endl;
	cout << "\tThroughput:" << endl;
	cout << "\tEDCA nodes (" << EDCAnodes << "): " << totalThroughputEDCA << endl;
	for(int i = 0; i < AC; i++)
	{
		cout << "\t\tAC " << i << ": " << totalACthroughputEDCA.at(i) << endl;
	}
	cout << "\t-CSMA/ECA nodes (" << EDCAnodes << "): " << totalThroughputECA << endl;
	for(int i = 0; i < AC; i++)
	{
		cout << "\t\tAC " << i << ": " << totalACthroughputECA.at(i) << endl;
	}

	cout << "\tAvg. Time between successful transmissions: " << endl;
	cout << "\t-EDCA nodes (" << EDCAnodes << "): " << endl;
	for(int i = 0; i < AC; i++)
	{
		cout << "\t\tAC " << i << ": " << (double)avgTimeBetweenACSxTxEDCA.at(i)/EDCAnodes << endl;
	}
	cout << "\t-CSMA/ECA_qos nodes (" << ECAnodes << "): " << endl;
	for(int i = 0; i < AC; i++)
	{
		cout << "\t\tAC " << i << ": " << (double)avgTimeBetweenACSxTxECA.at(i)/ECAnodes << endl;
	}

	cout << endl;
	cout << "\tFraction of collisions: " << endl;
	cout << "\t-EDCA nodes (" << EDCAnodes << "): " << endl;
	for(int i = 0; i < AC; i++)
	{
		cout << "\t\tAC " << i << ": " << (double)(totalACColEDCA.at(i)/EDCAnodes) << endl;
	}
	cout << "\t-CSMA/ECA_qos nodes (" << ECAnodes << "): " << endl;
	for(int i = 0; i < AC; i++)
	{
		cout << "\t\tAC " << i << ": " << (double)(totalACColECA.at(i)/ECAnodes) << endl;
	}		

	cout << "\n11. Last Collision: " << lastCollision << endl;

	cout << "\n12. Average queuing + contention time for each AC:" << endl;

	for(int i = 0; i < AC; i++)
	{
		cout << "\tAC " << i << ": " << avgQueueingDelay.at(i)/Nodes << endl;
	}
    
    cout << "\n13. Average final Backoff Stage (BK only)" << endl;
    cout << "\tDCF: " << avgBackoffStageDCF.at(0)/(Nodes*percentageEDCA_) << endl;
    cout << "\tECA: " << avgBackoffStageECA.at(0)/(Nodes*percentageEDCA_) << endl;

    cout << "\n14. Percetage of EDCA stations: " << percentageEDCA_*100 << "%" << endl;

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
	int howManyACs;
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
				cout << "4) Bandwidth: number of bits per second generated by the BK and BE sources. With 802.11n and EDCA, 10e6 < is considered an unsaturated environment." << endl;
				cout << "5) Batch: how many packets are put in the contenders queue. Used to simulate burst traffic. Usually set to 1" << endl;
				cout << "6) EDCA: using a deterministic backoff after successful transmissinos (0=no, 1=yes)" << endl;
				cout << "7) Stickiness: a deterministic backoff is used for this many number of collisions" << endl;
				cout << "8) FairShare: nodes at backoff stage k, attempt the transmission of 2^k packets (0=off, 1=on)" << endl;
				cout << "9) ChannelErrors: channel errors probability [0,1]" << endl;
				cout << "10) SlotDrift: probability of miscounting passing empty slots [0,1]" << endl;
				cout << "11) PercetageEDCA: percentage of nodes running EDCA. Used to simulate CSMA/ECA and CSMA/CA mixed scenarios [0,1]" << endl;
				cout << "12) MaxAggregation: nodes use maximum aggregation when attempting transmission (0=off, 1=on)" << endl;
				cout << "13) howManyACs: you can start simulations with 1 to 4 ACs" << endl;
				cout << "14) SimSeed: simulation seed used to generate random numbers. If testing results, repeat simulations with different seeds everytime" << endl << endl;
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
			cout << "./XXXX SimTime [10] NumNodes [2] PacketLength [1024] Bandwidth [65e6] Batch [1] ECA [0] Stickiness [0] fairShare [0] channelErrors [0] slotDrift [0] percentageOfEDCA [1] maxAggregation [0] howManyACs [0] simSeed [0]" << endl;
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
			howManyACs = 4; // default is CSMA/ECAqos with 4 ACs
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
		howManyACs = atoi(argv[13]); // 1 to 4 ACs.
		if(howManyACs == 0){
			howManyACs = 1;	
		}else if (howManyACs > 4)
		{
			howManyACs = 4;
		}
		simSeed = atof(argv[14]); //Simulation seed
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

	test.Setup(MaxSimIter,NumNodes,PacketLength,Bandwidth,Batch,Stickiness, ECA, fairShare, channelErrors, slotDrift, percentageEDCA, maxAggregation, howManyACs, simSeed);
	
	test.Run();


	return(0);
};
