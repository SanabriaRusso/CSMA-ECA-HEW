#!/bin/perl

use warnings;
use strict;

my @inputData;
my $highNodes = 0;
my $lowNodes = 10000;
my $outputFile = 'Results/slots_toPlot.txt';

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


print $fw 
("#1 Nodes,             #2 minimumCP,               #3 timeOfMinimumCP,
#4 slotCount,         #5 maximumCp,               #6 timeOfMaximumCP,
#7 slotCount\n");

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

    my $Cp = 3; #The column of Cp in the $filename
    my $times = 1;
    my $slotsPassed = 2;
    my ($min, $timeOfMin, $slotsToMin) = 1e6.00;
    my ($max, $timeOfMax, $slotsToMax) = 0.00;

    print $fw "$_ ";
    foreach my $i (0 .. $#{$metrics[$Cp]})
    {
        if ($min > ${$metrics[$Cp]}[$i])
        {
            $min = ${$metrics[$Cp]}[$i];
            $timeOfMin = ${$metrics[$times]}[$i];
            $slotsToMin = ${$metrics[$slotsPassed]}[$i];
        }

        if ($max < ${$metrics[$Cp]}[$i])
        {
            $max = ${$metrics[$Cp]}[$i];
            $timeOfMax = ${$metrics[$times]}[$i];
            $slotsToMax = ${$metrics[$slotsPassed]}[$i];
        }
    }
    print $fw ("$min $timeOfMin $slotsToMin $max $timeOfMax $slotsToMax\n");
}