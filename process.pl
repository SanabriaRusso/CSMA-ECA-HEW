#!/bin/perl
use warnings;
use strict;
use Data::Dumper qw(Dumper);
use Statistics::Basic qw(:all);
 
my @inputData;
my $highNodes = 0;
my $lowNodes = 10000;
my $outputFile = 'Results/processed_toPlot.txt';

####Finding the nodes limits based on the file.
my $filename = $ARGV[0];

open(my $fh, "<", $filename)
    or die "Could not open file '$filename' $!";

while (my $row = <$fh>)
{
    if($row !~ /^#/)
    {
        chomp($row);
        @inputData = split(/\s+/, $row);
        $highNodes = $inputData[0]
            if ($highNodes < $inputData[0]);
        $lowNodes = $inputData[0]
            if ($lowNodes > $inputData[0]);
    }

}

####Finding the metrics and outputting the results to file.
open(my $fw, ">", $outputFile)
    or die "Could not open write file $outputFile $!";

##Odd indexes are standard deviations if not defined
print $fw 
("#1 Nodes,                 #2 AvgThroughput,               #4 totalBKThroughput, 
#6 totalBEThroughput,       #8 totalVIThroughput,           #10 totalVOThroughput,
#12 totalCollisions,        #14 totalBKCollisions,          #16 totalBECollisions,
#18 totalVICollisions,      #20 TotalVOCollisions,          #22 totalInternalCollisions,    
#24 totalBKIntCol,          #26 totalBEIntCol,              #28 totalVIIntCol,
#30 totalVOIntCol,          #32 overallFairness,            #34 BKFairness,
#36 BEFairness,             #38 VIFairness,                 #40 VOFairness,
#42 avgTimeBtSxTxBK,        #44 avgTimeBtSxTxBE,            #46 avgTimeBtSxTxVI,
#48 avgTimeBtSxTxVO,        #50 qEmptyBK,                   #52 qEmptyBE,
#54 qEmptyVI,               #56 qEmptyVO                    #58 totalDropped,
#60 droppedBK,              #62 droppedBE                   #64 droppedVI,
#66 droppedVO               #68 channelErrors               #70 stickiness
#72 totalThroughputUnsat    #74 totalThroughputSat          #76 totalThroughputEDCA
#78 totalBKthroughputEDCA   #80 totalBEthroughputEDCA       #82 totalVIthroughputEDCA
#84 totalVOthroughputEDCA   #86 totalThroughputECA          #88 totalBKthroughputECA
#90 totalBEthroughputECA    #92 totalVIthroughputECA        #94 totalVOthroughputECA
#96 avgTimeBtSxTxBkEDCA     #98 avgTimeBtSxTxBeEDCA         #100 avgTimeBtSxTxViEDCA
#102 avgTimeBtSxTxVoEDCA    #104 avgTimeBtSxTxBkECA         #106 avgTimeBtSxTxBeECA
#108 avgTimeBtSxTxViECA     #110 avgTimeBtSxTxVoECA         #112 BKCollisionsEDCA
#114 BECollisionsEDCA       #116 VICollisionsEDCA           #118 VOCollisionsEDCA
#120 BKCollisionsECA        #122 BECollisionsECA            #124 VICollisionsECA
#126 VOCollisionsECA\n");

OUTTER: foreach($lowNodes .. $highNodes)
{
    open(my $fh, "<", $filename)
        or die "Could not open file $filename $!";

    my @metrics;
    my $thereIsData = 0;

    while (my $row = <$fh>)
    {
        if($row !~ /^#/)
        {
            chomp($row);
            @inputData = split(/\s+/, $row);
            if ($inputData[0] == $_)
            {
                $thereIsData = 1;
                foreach my $i (1 .. $#inputData)
                {
                    push @{$metrics[$i]}, $inputData[$i];    
                }
                
            }
        }
    }

    next OUTTER
        if($thereIsData == 0);

    my $average;
    my $std;
    print $fw "$_ ";
    foreach my $i (1 .. $#metrics)
    {
        $average = avg(@{$metrics[$i]}) + 0; #forcing the result to a scalar instead of an Object.
        $std = stddev(@{$metrics[$i]}) + 0;
        print $fw "$average $std ";
    }
    print $fw "\n";
}