#include <iostream>
#include <cstdlib>
#include <string>
#include <sstream>
#include <fstream>
#include "includes/lengthOfFile.hh"

using namespace std;

struct nodesAverage{
	int num;
	double rate;
	double cp;
	float JFI;
	double bandwidth;
	double delay;
	float backoffStage;
	double throughputDCF;
	double throughputECA;
	double sumDCFandECA;
	double Pb; //Blocking probability
	double dropped; //Dropped packets due retransmissions
	double sx; //successful slots
	double colSlots; //collision slots
	double empty; //empty slots
	double totalSlots;
	double tau;
	double avgQSize;
	double stdQSize;
	double avgQEmpty;
	double stdQEmpty;
};

int main(int argc, char *argv[])
{
    int N; //number of simulations per point
    int n_max;
    int n_min;
    int stickiness;
    int stageStickiness;
    int time;
    int fairShare;
    double b_min;
    float error;
    float drift;
    float DCF; //0-100
    int maxAggregation; //[0,1]
    
    if(argc < 11)
    {
    	if(argv[1])
    	{
    		string word = argv[1];
    		string help ("--help");
    		string helpShort ("-h");
    		if((word.compare(help) == 0) || (word.compare(helpShort)==0)){
    			cout << endl;
    			cout << "-----------------" << endl;
    			cout << "   Cheat Sheet:  " << endl;
    			cout << "-----------------" << endl;
    			cout << "(0)./XXXXX (1)Repetitions (2)N_max (3)N_min (4)Time (5)Rate (6)Stickiness (7)StageStickiness (8)FairShare (9)Error (10)SlotDrift (11)%DCF (12)MaxAggregation" << endl << endl;
    			return(0);
    		}else
    		{
    			cout << "Unintelligible command" << endl;
    			cout << "Use the --help or -h parameter to reveal the cheatsheet." << endl << endl;
    			return(0);
    		}
    	}else
    	{
    		cout << "Unintelligible command" << endl;
    		cout << "Use the --help or -h parameter to reveal the cheatsheet." << endl << endl;
    		return(0);
    	}
    }else
    {
    	N = atoi(argv[1]); //number of simulations per point
    	n_max = atoi(argv[2]);
        n_min = atoi(argv[3]);
 		time = atof(argv[4]);
		b_min = atof(argv[5]);
 		stickiness = atoi(argv[6]);
    	stageStickiness = atoi(argv[7]);
		fairShare = atoi(argv[8]);
		error = atof(argv[9]);
		drift = atof(argv[10]);
		DCF = atof(argv[11]); //0-100
		maxAggregation = atoi(argv[12]); //[0,1]
	}
    
    //For the statistics file
    //int statsLength = 0;
    nodesAverage *meanCarrier;
    //slotsAverage *slotCarrier;
    
    

    stringstream command (stringstream::in | stringstream::out);
    
    /*cout << "Enter the number of simulations per point" << endl;
    cin >> N;
    
    cout << "Please introduce n_max" << endl;
    cin >> n_max;
    cout << "Please introduce n_min" << endl;
    cin >> n_min;
    cout << "How long will the simulation run? (s)" << endl;
    cin >> time;
    cout << "Initial bandwidth?" << endl;
    cin >> b_min;
    cout << "Stickiness?" << endl;
    cin >> stickiness;
    cout << "Stage Stickiness?" << endl;
    cin >> stageStickiness;
    cout << "fairShare?" << endl;
    cin >> fairShare;
    cout << "Channel error prob (%)" << endl;
    cin >> error;
    cout << "Slot drift (%)" << endl;
    cin >> drift;
    cout << "Percentage of nodes executing DCF (%)" << endl;
    cin >> DCF;
    cout << "Maximum Aggregation? (0=no, 1=yes)" << endl;
    cin >> maxAggregation;*/
    
    for(int i = n_min; i <= n_max; i++)
    {
        for(int j = 0; j < N; j++)
        {
            //execute script  
            command << "/home/lsr/Dropbox/PhD/Research/HEW/CSMA-ECA/Sim_SlottedCSMA" << " " << time << " " << i << " 1024 " << b_min << " 1 " << stickiness << " " << stageStickiness << " " << fairShare << " " << error/100 << " " << drift/100  << " " << DCF/100 << " " << maxAggregation << " " << j << endl;
            //command << "/Users/L_SR/Dropbox/PhD/NeTS/git/CSMA-E2CA/Sim_SlottedCSMA" << " " << time << " " << i << " 1024 " << b_min << " 1 " << stickiness << " " << stageStickiness << " " << fairShare << " " << error/100 << " " << drift/100  << " " << DCF/100 << " " << maxAggregation << " " << j << endl;
            //cout << command.str() << endl;
            cout << endl;
            cout << "Trying with " << i << " stations." << endl << endl;
            system(command.str().c_str()); 
            command.str("");
            //cout << "Time: " << time << " Station: " << i << " Packet Size: 1024" << " Rate: " << b_min << " Batch: 1 " << "Stickiness: " << stickiness << " stageStickiness: " << stageStickiness << " FairShare: " << fairShare << " Error: " << error/100 << " Drift: " << drift/100 << " DCF: " << DCF/100 << " MA: " << maxAggregation << " Repetition: " << j << endl;
        }
    }
    
    
    //Manipulating the statistics file
    ifstream fin("Results/multiSim.txt");
    string line;
    meanCarrier = new nodesAverage[N];
    
    //statsLength = lengthOfFile(&fin);    
    
    //cout << statsLength << endl;	
    
    ifstream inputFile("Results/multiSim.txt");
    string input;
    int iterator = 0;
    
	ofstream multiAverage;
	multiAverage.open("Results/multiAverage.txt", ios::app);
	multiAverage << "#1-sta 2-avgThroughput 3-stdThroughput 4-JFI 5-stdJFI 6-Bandwidth 7-avgDelay 8-stdDelay 9-avgBackoffStage 10-stdBackoffStage 11-totalFractionOfCollisionSlots 12.stdCollidingSlots 13.avgThroughputDCF 14. stdThroughputDCF 15.avgThroughputECA 16.stdThroughputECA 17.Sum of 15 and 16 18. std of 17 19. AvgBlockingProb 20. StdBlockingProb 21. AvgDroppedPackets 22.StdDropped 23.SxSlots 24.stdSxSlots 25.ColSlots 26.StdColSlots 27.EmptySlots 28.stdEmpty 29.totalSlots 30.stdTotalSlots 31.averageTAU 32.stdTAU 33.avgQSize 34.stdQSize 35.avgQEmpty 36.stdQEmpty" << endl;
    
    while(getline(inputFile,input))
    {
    	if(iterator != N-1)
    	{
    		istringstream tokenizer(input);
    		string token;
    	
	    	getline(tokenizer, token, ' ');
    		istringstream stations(token);
	    	int s;
    		stations >> s;
	    	//cout << s << endl;
    		meanCarrier[iterator].num = s;
    	
	    	getline(tokenizer, token, ' ');
    		istringstream rate(token);
	    	double r;
    		rate >> r;
	    	//cout << r << endl;
    		meanCarrier[iterator].rate = r;
    		
    		getline(tokenizer, token, ' ');
    		istringstream colProb(token);
	    	double cp;
    		colProb >> cp;
	    	//cout << cp << endl;
    		meanCarrier[iterator].cp = cp;
    		
    		getline(tokenizer, token, ' ');
    		istringstream jainsIndex(token);
	    	double jfi;
    		jainsIndex >> jfi;
	    	//cout << jfi << endl;
    		meanCarrier[iterator].JFI = jfi;
    		
    		getline(tokenizer, token, ' ');
    		istringstream bandwidth(token);
	    	double bw;
    		bandwidth >> bw;
	    	//cout << bw << endl;
    		meanCarrier[iterator].bandwidth = bw;
    		
    		getline(tokenizer, token, ' ');
    		istringstream delay(token);
	    	double sysDelay;
    		delay >> sysDelay;
	    	//cout << sysDelay << endl;
    		meanCarrier[iterator].delay = sysDelay; 

    		getline(tokenizer, token, ' ');
    		istringstream backoffStage(token);
	    	double avgBackoffStage;
    		backoffStage >> avgBackoffStage;
	    	//cout << avgBackoffStage << endl;
    		meanCarrier[iterator].backoffStage = avgBackoffStage;
    		
    		getline(tokenizer, token, ' ');
    		istringstream DCF(token);
	    	double throughputDCF;
    		DCF >> throughputDCF;
	    	//cout << throughputDCF << endl;
    		meanCarrier[iterator].throughputDCF = throughputDCF;
    		
    		getline(tokenizer, token, ' ');
    		istringstream ECA(token);
	    	double throughputECA;
    		ECA >> throughputECA;
	    	//cout << throughputECA << endl;
    		meanCarrier[iterator].throughputECA = throughputECA;
    		
    		getline(tokenizer, token, ' ');
    		istringstream sum(token);
	    	double sumDCFandECA;
    		sum >> sumDCFandECA;
	    	//cout << sumDCFandECA << endl;
    		meanCarrier[iterator].sumDCFandECA = sumDCFandECA;
    		
    		getline(tokenizer, token, ' ');
    		istringstream Pb(token);
	    	double blockingProbability;
    		Pb >> blockingProbability;
	    	//cout << blockingProbability << endl;
    		meanCarrier[iterator].Pb = blockingProbability;
    		
    		getline(tokenizer, token, ' ');
    		istringstream drop(token);
	    	double droppedPackets;
    		drop >> droppedPackets;
	    	//cout << droppedPackets << endl;
    		meanCarrier[iterator].dropped = droppedPackets;
    		
    		getline(tokenizer, token, ' ');
    		istringstream sx(token);
	    	double successSlots;
    		sx >> successSlots;
	    	//cout << successSlots << endl;
    		meanCarrier[iterator].sx = successSlots;
    		
    		getline(tokenizer, token, ' ');
    		istringstream cS(token);
	    	double colSlots;
    		cS >> colSlots;
	    	//cout << colSlots << endl;
    		meanCarrier[iterator].colSlots = colSlots;
    		
    		getline(tokenizer, token, ' ');
    		istringstream empty(token);
	    	double emptySlots;
    		empty >> emptySlots;
	    	//cout << emptySlots << endl;
    		meanCarrier[iterator].empty = emptySlots;
    		
    		getline(tokenizer, token, ' ');
    		istringstream total(token);
	    	double totalSlots;
    		total >> totalSlots;
	    	//cout << totalSlots << endl;
    		meanCarrier[iterator].totalSlots = totalSlots;
    		
    		getline(tokenizer, token, ' ');
    		istringstream Pt(token);
	    	double tau;
    		Pt >> tau;
	    	//cout << tau << endl;
    		meanCarrier[iterator].tau = tau;
    		
    		getline(tokenizer, token, ' ');
    		istringstream q(token);
	    	double qSize;
    		q >> qSize;
	    	//cout << qSize << endl;
    		meanCarrier[iterator].avgQSize = qSize;
    		
    		getline(tokenizer, token, ' ');
    		istringstream qE(token);
	    	double qEmpty;
    		qE >> qEmpty;
	    	//cout << qEmpty << endl;
    		meanCarrier[iterator].avgQEmpty = qEmpty;
    	
    		iterator++;
    	}else
    	{
    		double numerator = 0;
    		double numerator2 = 0;
    		double numeratorJFI = 0;
    		double numeratorDelay = 0;
    		double numeratorSTDelay = 0;
    		double average = 0;
    		double stDeviation = 0.0;
    		double stDeviationDelay = 0.0;
    		double avgJFI = 0;
    		double stdJFI = 0;
    		double avgDelay = 0;
    		double avgBOStage = 0;
    		double numSTDBOS = 0;
    		double stdBOS = 0;
    		double avgCP = 0;
    		double numCP = 0;
    		double stdCP = 0;
    		double avgThroughputDCF = 0;
    		double avgThroughputECA = 0;
    		double stdThroughputDCF = 0;
    		double stdThroughputECA = 0;
    		double avgSumThroughput = 0;
    		double stdSumThroughput = 0;
    		double avgPb = 0;
    		double stdPb = 0;
    		double avgDropped = 0;
    		double stdDropped = 0;
    		double avgSxSlots = 0;
    		double stdSxSlots = 0;
    		double avgColSlots = 0;
    		double stdColSlots = 0;
    		double avgEmptySlots = 0;
    		double stdEmptySlots = 0;
    		double avgTotalSlots = 0;
    		double stdTotalSlots = 0;
    		double avgTau = 0;
    		double stdTau = 0;
    		double avgQSize = 0;
    		double stdQSize = 0;
    		double avgQEmpty = 0;
    		double stdQEmpty = 0;
    		
    		
    		//Computing the averages
    		for(int i = 0; i <= iterator; i++)
    		{
    			numerator += meanCarrier[i].rate;
    			numeratorJFI += (meanCarrier[i].JFI);
    			numeratorDelay += (meanCarrier[i].delay);
    			avgBOStage += (meanCarrier[i].backoffStage);
    			avgCP += (meanCarrier[i].cp);
    			avgThroughputDCF += (meanCarrier[i].throughputDCF);
    			avgThroughputECA += (meanCarrier[i].throughputECA);
    			avgSumThroughput += (meanCarrier[i].sumDCFandECA);
    			avgPb += (meanCarrier[i].Pb);
    			avgDropped += (meanCarrier[i].dropped);
    			avgSxSlots += (meanCarrier[i].sx);
    			avgColSlots += (meanCarrier[i].colSlots);
    			avgEmptySlots += (meanCarrier[i].empty);
    			avgTotalSlots += (meanCarrier[i].totalSlots);
    			avgTau += (meanCarrier[i].tau);
    			avgQSize += (meanCarrier[i].avgQSize);
    			avgQEmpty += (meanCarrier[i].avgQEmpty);
    		}
    		average = numerator/iterator;
    		avgJFI = numeratorJFI/iterator;
    		avgDelay = numeratorDelay/iterator;
    		avgBOStage /= iterator;
    		avgCP /= iterator;
    		avgThroughputDCF /= iterator;
    		avgThroughputECA /= iterator;
    		avgSumThroughput /= iterator;
    		avgPb /= iterator;
    		avgDropped /= iterator;
    		avgSxSlots /= iterator;
    		avgColSlots /= iterator;
    		avgEmptySlots /= iterator;
    		avgTotalSlots /= iterator;
    		avgTau /= iterator;
    		avgQSize /= iterator;
    		avgQEmpty /= iterator;
    		
    		
    		
    		//Computing the standard deviation for the error bars
    		numeratorJFI = 0;
    		for(int j = 0; j <= iterator; j++)
    		{	
    			if(meanCarrier[j].rate > 0)
    			{
    				numerator2 += pow(meanCarrier[j].rate - average,2);
    				numeratorSTDelay += pow(meanCarrier[j].delay - avgDelay,2);
    				numeratorJFI += pow((meanCarrier[j].JFI) - avgJFI,2);
    				numSTDBOS += pow((meanCarrier[j].backoffStage) - avgBOStage,2);
    				numCP += pow((meanCarrier[j].cp) - avgCP,2);
    				stdThroughputDCF += pow((meanCarrier[j].throughputDCF) - avgThroughputDCF,2);
    				stdThroughputECA += pow((meanCarrier[j].throughputECA) - avgThroughputECA,2);
    				stdSumThroughput += pow((meanCarrier[j].sumDCFandECA) - avgSumThroughput,2);
    				stdPb += pow((meanCarrier[j].Pb) - avgPb,2);
    				stdDropped += pow((meanCarrier[j].dropped) - avgDropped,2);
    				stdSxSlots += pow((meanCarrier[j].sx) - avgSxSlots,2);
    				stdColSlots += pow((meanCarrier[j].colSlots) - avgColSlots,2);
    				stdEmptySlots += pow((meanCarrier[j].empty) - avgEmptySlots,2);
    				stdTotalSlots += pow((meanCarrier[j].totalSlots) - avgTotalSlots,2);
    				stdTau += pow((meanCarrier[j].tau) - avgTau,2);
    				stdQSize += pow((meanCarrier[j].avgQSize) - avgQSize,2);
    				stdQEmpty += pow((meanCarrier[j].avgQEmpty) - avgQEmpty,2);
    			}
    		}
    		
    		stDeviation = sqrt((1./(iterator))*numerator2);
    		stDeviationDelay = sqrt((1./(iterator))*numeratorSTDelay);
    		stdJFI = sqrt((1./(iterator))*numeratorJFI);
    		stdBOS = sqrt((1./(iterator))*numSTDBOS);
    		stdCP = sqrt((1./(iterator))*numCP);
    		stdThroughputDCF = sqrt((1./(iterator))*stdThroughputDCF);
    		stdThroughputECA = sqrt((1./(iterator))*stdThroughputECA);
    		stdSumThroughput = sqrt((1./(iterator))*stdSumThroughput);
    		stdPb = sqrt((1./(iterator))*stdPb);
    		stdDropped = sqrt((1./(iterator))*stdDropped);
    		stdSxSlots = sqrt((1./(iterator))*stdSxSlots);
    		stdColSlots = sqrt((1./(iterator))*stdColSlots);
    		stdEmptySlots = sqrt((1./(iterator))*stdEmptySlots);
    		stdTotalSlots = sqrt((1./(iterator))*stdTotalSlots);
    		stdTau = sqrt((1./(iterator))*stdTau);
    		stdQSize = sqrt((1./(iterator))*stdQSize);
    		stdQEmpty = sqrt((1./(iterator))*stdQEmpty);
    		    		
    		multiAverage << meanCarrier[iterator-1].num << " " << average << " " << stDeviation << " " << avgJFI << " " << stdJFI << " " << meanCarrier[iterator-1].bandwidth << " " << avgDelay << " " << stDeviationDelay << " " << avgBOStage << " " << stdBOS << " " << avgCP << " " << stdCP << " " << avgThroughputDCF << " " << stdThroughputDCF << " " << avgThroughputECA << " " << stdThroughputECA << " " << avgSumThroughput << " " << stdSumThroughput << " " << avgPb << " " << stdPb << " " << avgDropped << " " << stdDropped << " " << avgSxSlots << " " << stdSxSlots << " " << avgColSlots << " " << stdColSlots << " " << avgEmptySlots << " " << stdEmptySlots << " " << avgTotalSlots << " " << stdTotalSlots << " " << avgTau << " " << stdTau << " " << avgQSize << " " << stdQSize << " " << avgQEmpty << " " << stdQEmpty << endl;
    		iterator = 0;
    		
    	}
    }//for the while statement
    
    multiAverage.close();
    
    
    //---------------------------------
    //Processing per nodes statistics
    //When n_max-n_min=0
    //---------------------------------
    
    //Manipulating the statistics file
    if(n_max - n_min == 0)
    {
    	nodesAverage** staMeanCarrier = new nodesAverage*[n_max];
    	for(int i = 0; i < n_max; i++)
    	{
    		staMeanCarrier[i] = new nodesAverage[N];
    	}
    
    	ifstream staInputFile("Results/multiStation.txt");
    
		ofstream staMultiAverage;
		staMultiAverage.open("Results/staMultiAverage.txt", ios::app);
		staMultiAverage << "#1-sta 2-avgThroughput 3-stdThroughput 4-avgPc 5-stdPc 6-avgTAU 7-stdTAU 8-avgDelay 9-stdDelay 10-avgEmpty 11-stdEmpty 12-avgQSize 13-stdQSize 14-avgBackoffStage 15-stdBackoffStage 16-avgDropped 17-stdDropped" << endl;
    
    	if(!staInputFile)
    	{
        	cout << "File could not be opened" << endl;
    	}else
    	{
    		for(int r = 0; r < N ; r++)
			{
				for(int s = 0; s < n_max; s++)
				{
            		staInputFile >> staMeanCarrier[s][r].num >> staMeanCarrier[s][r].rate >> staMeanCarrier[s][r].cp >> staMeanCarrier[s][r].tau >> staMeanCarrier[s][r].delay >> staMeanCarrier[s][r].avgQEmpty >> staMeanCarrier[s][r].avgQSize >> staMeanCarrier[s][r].backoffStage >> staMeanCarrier[s][r].dropped;
        		}
        	}
	    }

		//Calculating the average and std
		double avgStaRate = 0;
		double stdStaRate = 0;
		double avgStaPc = 0;
		double stdStaPc = 0;
		double avgStaTAU = 0;
		double stdStaTAU = 0;
		double avgStaDelay = 0;
		double stdStaDelay = 0;
		double avgStaQEmpty = 0;
		double stdStaQEmpty = 0;
		double avgStaQSize = 0;
		double stdStaQSize = 0;
		double avgStaBackoffStage = 0;
		double stdStaBackoffStage = 0;
		double avgStaDropped = 0;
		double stdStaDropped = 0;
		
		for(int i = 0; i < n_max; i++)
		{
			for(int j = 0; j < N; j++)
			{
				avgStaRate += staMeanCarrier[i][j].rate;
				avgStaPc += staMeanCarrier[i][j].cp;
				avgStaTAU += staMeanCarrier[i][j].tau;
				avgStaDelay += staMeanCarrier[i][j].delay;
				avgStaQEmpty += staMeanCarrier[i][j].avgQEmpty;
				avgStaQSize += staMeanCarrier[i][j].avgQSize;
				avgStaBackoffStage += staMeanCarrier[i][j].backoffStage;
				avgStaDropped += staMeanCarrier[i][j].dropped;
			}
			
			avgStaRate /= N;
			avgStaPc /= N;
			avgStaTAU /= N;
			avgStaDelay /= N;
			avgStaQEmpty /= N;
			avgStaQSize /= N;
			avgStaBackoffStage /= N;
			avgStaDropped /= N;
			
			//Calculating the STD
			for(int j = 0; j < N; j++)
			{
				stdStaRate += pow((staMeanCarrier[i][j].rate) - avgStaRate,2);
				stdStaPc += pow((staMeanCarrier[i][j].cp) - avgStaPc,2);
				stdStaTAU += pow((staMeanCarrier[i][j].tau) - avgStaTAU,2);
				stdStaDelay += pow((staMeanCarrier[i][j].delay) - avgStaDelay,2);
				stdStaQEmpty += pow((staMeanCarrier[i][j].avgQEmpty) - avgStaQEmpty,2);
				stdStaQSize += pow((staMeanCarrier[i][j].avgQSize) - avgStaQSize,2);
				stdStaBackoffStage += pow((staMeanCarrier[i][j].backoffStage) - avgStaBackoffStage,2);
				stdStaDropped += pow((staMeanCarrier[i][j].dropped) - avgStaDropped,2);
			}
			stdStaRate = sqrt((1./(N))*stdStaRate);
			stdStaPc = sqrt((1./(N))*stdStaPc);			
			stdStaTAU = sqrt((1./(N))*stdStaTAU);
			stdStaDelay = sqrt((1./(N))*stdStaDelay);
			stdStaQEmpty = sqrt((1./(N))*stdStaQEmpty);
			stdStaQSize = sqrt((1./(N))*stdStaQSize);
			stdStaBackoffStage = sqrt((1./(N))*stdStaBackoffStage);
			stdStaDropped = sqrt((1./(N))*stdStaDropped);
			
			//Writing in the output file
			staMultiAverage << i << " " << avgStaRate << " " << stdStaRate << " " << avgStaPc << " " << stdStaPc << " " << avgStaTAU << " " << stdStaTAU << " " << avgStaDelay << " " << stdStaDelay << " " << avgStaQEmpty << " " << stdStaQEmpty << " " << avgStaQSize << " " << stdStaQSize << " " << avgStaBackoffStage << " " << stdStaBackoffStage << endl;
			
			avgStaRate = 0;
			stdStaRate = 0;
			avgStaPc = 0;
			stdStaPc = 0;
			avgStaTAU = 0;
			stdStaTAU = 0;
			avgStaDelay = 0;
			stdStaDelay = 0;
			avgStaQEmpty = 0;
			stdStaQEmpty = 0;
			avgStaQSize = 0;
			stdStaQSize = 0;
			avgStaBackoffStage = 0;
			stdStaBackoffStage = 0;
			avgStaDropped = 0;
			stdStaDropped = 0;
		}

    	staMultiAverage.close();
    }
}
