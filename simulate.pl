#!/bin/perl

use warnings;
use strict;
use Switch;

die ("Getting out. You need some parameters, i.e.: --help\n") if (not exists $ARGV[0]);

chomp($ARGV[0]);

my %help = (
				help		=> 1,
				'-h'		=> 1,
				'--help'	=> 1
			);

die ("******Help\n", "ARGV. field:\n", "0. Repetitions 1. Time 2. Nmax 3. Nmin 4. Jumps 5. Bandwidth 6. Channel errors 7. EDCA share 8. ECA Code 9. numACs 10. LENGTH(Bytes)\n") 
	if (exists $help{$ARGV[0]});

my $rep = $ARGV[0];
my $time = $ARGV[1];
my $Nmax = $ARGV[2];
my $Nmin = $ARGV[3];
my $jump = $ARGV[4];
my $bandwidth = $ARGV[5];
my $errors = $ARGV[6];
my $EDCA_share = $ARGV[7];
my $ECA = $ARGV[8];
my $numACs = $ARGV[9];
my $length = $ARGV[10];
my $batch = 1;
my $drift = 0;
my $maxAggregation = 0;

my $stickiness = 0;
my $fairShare = 0;

print ("Going to simulate:\n");
if ($Nmax == $Nmin){
	print ("\t$rep repetitions of $time seconds, ", $Nmin, " stations, $bandwidth Mbps, errors $errors and ECA $ECA with $numACs ACs.\n\n\n");	
}else{
	print ("\t$rep repetitions jumping $jump of $time seconds, ", ($Nmax - $Nmin), " stations, $bandwidth Mbps, errors $errors and ECA $ECA with $numACs ACs.\n\n\n");
}

switch ($ECA){
	case 0 {
		$stickiness = 0;
		$fairShare = 0;
		
	}
	case 1 {
		$stickiness = 1;
		$fairShare = 0;
		
	}
	case 2 {
		$ECA = 1;
		$stickiness = 0;
		$fairShare = 0;
		
	}
	case 3 {
		$ECA = 1; 
		$stickiness = 2;
		$fairShare = 0;
		
	}
	case 4 {
		$ECA = 1;
		$stickiness = 2;
		$fairShare = 1;
	}
	case 5 {
		$ECA = 1;
		$stickiness = 2;
		$fairShare = 1;
	}
	case 6 {
		$ECA = 0;
		$stickiness = 0;
		$fairShare = 0;
		$maxAggregation = 1 ;
	}
	case 7 {
		$ECA = 0;
		$stickiness = 0;
		$fairShare = 1;
		$maxAggregation = 0;
	}
}

my $compile = './build_local';
my @command;
my @jumps;

system($compile);
die "Command failed\n" if ($? != 0);

#Simulating at $jump intervals
foreach ($Nmin .. $Nmax)
{
	push @jumps, $_
		if $_ % $jump == 0;

}
push @jumps, $Nmax
	if $jumps[-1] != $Nmax;


OUTTER: foreach my $i (@jumps){
	INNER: foreach my $j (1 .. $rep){
		my $seed = int(rand()*1000);
		@command = ("./ECA_exec $time $i $length $bandwidth $batch $ECA $stickiness $fairShare $errors $drift $EDCA_share $maxAggregation $numACs $seed"); 
		print ("\n\n****Node #$i of $Nmax ($?).\n");
		print ("****Iteration #$j of $rep.\n");
		print ("**** @command\n");
		system(@command);
		(print ("\n\n********Execution failed\n\tQuitting iterations\n") and last OUTTER) if ($? != 0);
	}
}


#Calling the parser
my $parserFile = 'process.pl';
my $parseSlots = 'analyseSlots.pl';
my $dataFile = 'Results/output.txt';
my $slotsFile = 'Results/slotsInTime.txt';
my @parseCommand = ("./$parserFile $dataFile");
system(@parseCommand);
(print ("\n\n********Processing failed\n") and last OUTTER) if ($? != 0);
@parseCommand = ("./$parseSlots $slotsFile");
system(@parseCommand);
(print ("\n\n********Processing failed\n") and last OUTTER) if ($? != 0);
