#include <math.h>
#include <iostream>

using namespace std;

double stats(int successful_slots, int empty_slots, int collision_slots, int payload){
    
    double Ts, Tc;
    double throughput;
    
    // MAC Parameters (802.11b specifications)
    const double PHY_RATE = 1E6;
    const double RBASIC = 1E6;
    const double RDATA = 11E6;
    const double PCLP_PREAMBLE = 144; // bits (Long preamble)
    const double PCLP_HEADER = 48; // bits
    //const double PHY_HEADER = PCLP_PREAMBLE + PCLP_HEADER;
    const double MAC_HEADER = 272; // bits (including the CRC)
    const double L_ACK = 112; // bits
    //const double RTS = 160; // bits
    //const double CTS = 112; // bits
    const double SLOT_TIME = 20E-6;
    const double DIFS2 = 50E-6;
    const double SIFS2 = 10E-6;
    const double EIFS2 = SIFS2 + DIFS2 + ((144 + 48 + 112)/RBASIC);

    //L = 1500*8;
    const double L = payload*8;
    
    
    
    Ts = ((PCLP_PREAMBLE + PCLP_HEADER)/PHY_RATE) + ((L + MAC_HEADER)/RDATA) + SIFS2 + ((PCLP_PREAMBLE + PCLP_HEADER)/PHY_RATE) + (L_ACK/RBASIC) + DIFS2;
    Tc = ((PCLP_PREAMBLE + PCLP_HEADER)/RBASIC)+ ((L+MAC_HEADER)/RDATA) + EIFS2;
    
    //cout << "Ts: " << Ts << endl;
    //cout << "Tc: " << Tc << endl;
    
    throughput = successful_slots*L/(empty_slots*SLOT_TIME+successful_slots*Ts+collision_slots*Tc);
    
    return(throughput);
}
