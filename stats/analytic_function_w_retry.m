function throughput = analytic_function_w_retry(number_of_stations,CWmin,m,mr)

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

   Ts
   Tc


    W=CWmin;
    n=number_of_stations;
    tau = fzero(@(tau) fun_ztau_w_retry(tau,n,W,m,mr), 0.1);
    pe=(1-tau)^n
    ps=n*tau*(1-tau)^(n-1)
    pc=1-ps-pe
    throughput=ps*L/(pe*SLOT_TIME+ps*Ts+pc*Tc);

    %See that the channel object in the simulator. It has all the pe, ps and pc.
    
endfunction
