function throughput = thoughput_estimation(number_of_stations,pempty,psuccess,pcollision)

% MAC Parameters (802.11b specifications)
   PHY_RATE = 1E6;
   RBASIC = 1E6;
   RDATA = 11E6;
   PCLP_PREAMBLE=144; % bits (Long preamble)
   PCLP_HEADER=48; % bits
   PHY_HEADER=PCLP_PREAMBLE+PCLP_HEADER;
   MAC_HEADER=272; % bits (including the CRC)
   L_ACK=112; %bits
   RTS=160; %bits
   CTS=112; %bits
   DIFS=50E-6;
   SIFS=10E-6;
   SLOT_TIME=20E-6;
   EIFS=SIFS+DIFS+((144+48+112)/RBASIC);

   L=1024*8;
   Ts=((PCLP_PREAMBLE+PCLP_HEADER)/PHY_RATE)+((L+MAC_HEADER)/RDATA)+SIFS+((PCLP_PREAMBLE+PCLP_HEADER)/PHY_RATE)+(L_ACK/RBASIC)+DIFS;
   Tc=((PCLP_PREAMBLE+PCLP_HEADER)/RBASIC)+((L+MAC_HEADER)/RDATA)+EIFS;



    W=32;
    n=number_of_stations;
    pe=pempty
    ps=psuccess
    pc=pcollision
    throughput=ps*L/(pe*SLOT_TIME+ps*Ts+pc*Tc);

    %See that the channel object in the simulator. It has all the pe, ps and pc.
    
endfunction
