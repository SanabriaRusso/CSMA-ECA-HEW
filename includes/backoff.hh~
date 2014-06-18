#include <time.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <math.h>
#include <algorithm>

#define CWMIN 16 //should be the same as ../STA.h

using namespace std;

int backoff(int backoff_stage, int stickiness, float driftProbability){

	int backoff_counter = 0;
	
	if(stickiness != 0){
		backoff_counter = (int)(pow(2,backoff_stage)*CWMIN/2)-1;
	}else{
		backoff_counter = rand() % (int)(pow(2,backoff_stage)*CWMIN);
	}
	
	//cout << "Backoff before slot drift: " << backoff_counter << endl;
	
	//Determining the slot drift according to the probability
	
	//if driftProbability = p, then with p/2 it will lead a slot and with p/2 it will lag a slot
	//cout << driftProbability << endl;
	if(driftProbability > 0){
	    float slotDrift = rand() % 100 + 1;
	    slotDrift/=100;
	    //cout << slotDrift << endl;
	    if((backoff_counter > 0) && (slotDrift > 0) && (slotDrift <= driftProbability/2.))
	    {
	        backoff_counter--; //leads one slot
	        //cout << "Leads one slot" << endl;
	    }else if((slotDrift > driftProbability/2.) && (slotDrift <= driftProbability))
	    {
	        backoff_counter++; //lags one slot
	        //cout << "Lags one slot" << endl;
	    }
	}
	return (backoff_counter);
}