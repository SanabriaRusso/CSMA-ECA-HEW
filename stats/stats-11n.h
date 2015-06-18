#include <math.h>
#include <iostream>

using namespace std;

double stats11n(int successful_slots, int empty_slots, int collision_slots, int payload, int aggregation){
    
    double Ts, Tc, throughput;
    double Tsymbol, PHYn, LDrate, SIFSn, DIFSn, TBack, Tempty, slotTime;
    int L, LData, LBack;
    
    Tsymbol = 4e-06; //symbol time
    PHYn = 32e-06; //PHY header duration
    L = payload;
    LData = 288 + (8*L);
    LDrate = 260 //bits. This is the max. The min is 26
    SIFSn = 9e-06;
    DIFSn = 34e-06;
    Tempty = 9e-06;
    slotTime = 16e-06;
    LBack = 256; //length of the block acknowledgement
    TBack = 32e-06 + ceil((16e-06+LBack+6e-06)/LDrate) * Tsymbol;
    
    
    Ts = 32e-06 + ceil((16e-06 + aggregation*(32e-06 + LData)+6e-06)/LDrate) * Tsymbol + SIFSn + TBack + DIFSn + Tempty;
    Tc = Ts;
    
    throughput = successful_slots*L/(empty_slots*slotTime+successful_slots*Ts+collision_slots*Tc);
    
    return(throughput);
}
