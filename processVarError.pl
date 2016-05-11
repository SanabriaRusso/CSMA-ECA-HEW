#!/usr/local/bin/perl
use warnings;
use strict;
use Data::Dumper qw(Dumper);
use Statistics::Basic qw(:all);
 
my @inputData;
my $highError = 0;
my $lowError = 10000;
my $outputFile = 'Results/processed_varError.txt';

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
        $highError = $inputData[34]
            if ($highError < $inputData[34]);
        $lowError = $inputData[34]
            if ($lowError > $inputData[34]);
    }

}
####Normalizing the errors
$highError *= 100;
$lowError *= 100;

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
#126 VOCollisionsEDCA       #128 lastCollision              #130 avgQueuingDelayBK
#132 avgQueuingDelayBE      #134 avgQueuingDelayVI          #136 avgQueuingDelayVO
#138 avgBackoffStageECABK   #140 avgBackoffStageDCFBK       #142 PercentageEDCA
#144 sxSlots                #146 colSlots                   #148 errorSlots
#150 emptySlots\n");


OUTTER: foreach($lowError .. $highError)
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
            if ($inputData[34] == ($_/100))
            {
                $thereIsData = 1;
                foreach my $i (1 .. $#inputData)
                {
                    push @{$metrics[$i]}, $inputData[$i];    
                }
                # print ("@inputData\n\n");
            }
        }
    }

    next OUTTER
        if($thereIsData == 0);

    my $average;
    my $std;
    print $fw "$inputData[0] ";
    foreach my $i (1 .. $#metrics)
    {
        $average = avg(@{$metrics[$i]}) + 0; #forcing the result to a scalar instead of an Object.
        $std = stddev(@{$metrics[$i]}) + 0;
        print $fw "$average $std ";
        # print ("$average $std\n");
    }
    print $fw "\n";
}