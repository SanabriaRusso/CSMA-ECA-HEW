function throughput = analytic_function(number_of_stations,CWmin,m)

% MAC Parameters (802.11b specifications)
    PACKET_PAYLOAD = 8148;
    MAC_HEADER = 272;
    PHY_HEADER = 128;
    ACK = 112 + PHY_HEADER;
    CHANNEL_BITRATE = 1E6;
    PROPAGATION_DELAY = 1E-6;
    SLOT_TIME = 50E-6;
    SIFS = 28E-6;
    DIFS = 128E-6;
    ACK_TIMEOUT = 300E-6;
    CTS_TIMEOUT = 300E-6;

    Ts = (PHY_HEADER + MAC_HEADER + PACKET_PAYLOAD)/CHANNEL_BITRATE + SIFS + PROPAGATION_DELAY + ACK/CHANNEL_BITRATE + DIFS + PROPAGATION_DELAY;
    Tc = (PHY_HEADER + MAC_HEADER + PACKET_PAYLOAD)/CHANNEL_BITRATE + DIFS + PROPAGATION_DELAY;


    W=CWmin;
    n=number_of_stations;
    tau = fzero(@(tau) fun_ztau(tau,n,W,m), 0.1);
    pe=(1-tau)^n;
    ps=n*tau*(1-tau)^(n-1);
    pc=1-ps-pe;

    throughput=ps*PACKET_PAYLOAD/(pe*SLOT_TIME+ps*Ts+pc*Tc);
    
endfunction
