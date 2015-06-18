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

die ("******Help\n", "ARGV. field:\n", "\t0. Repetitions\n \t1. Time\n \t2. Nodes\n \t3. Bandwidth\n \t4. %Channel errors min\n \t5. %Channel errors max\n \t6. %Jumps\n \t7. ECA Code\n") 
	if (exists $help{$ARGV[0]});

my $rep = $ARGV[0];
my $time = $ARGV[1];
my $N = $ARGV[2];
my $bandwidth = $ARGV[3];
my $erMin = $ARGV[4];
my $erMax = $ARGV[5];
my $jump = $ARGV[6];
my $ECA = $ARGV[7];
my $length = 1024;
my $batch = 1;
my $drift = 0;
my $EDCA = 0;
my $maxAggregation = 0;
my $stickiness = 0;
my $fairShare = 0;

print ("Going to simulate:\n");
if ($erMax == $erMin){
	print ("\t$rep repetitions of $time seconds, $N stations, $bandwidth Mbps, %errors $erMax and ECA $ECA.\n\n\n");	
}else{
	print ("\t$rep repetitions jumping $jump of $time seconds, ", ($erMax - $erMin), " %channel errors gap, $N stations, $bandwidth Mbps and ECA $ECA.\n\n\n");
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
		$stickiness = 1;
		$fairShare = 1;
		
	}#BASIC ECA
	case 3 {
		$ECA = 1; 
		$stickiness = 0;
		$fairShare = 0;
		
	}
	case 4 {
		$ECA = 1;
		$stickiness = 5;
		$fairShare = 1;
	}
	case 5 {
		$ECA = 1;
		$stickiness = 1000;
		$fairShare = 1;
	}
}

my $compile = './build_local';
my @command;
my @jumps;

system($compile);
die "Command failed\n" if ($? != 0);

#Simulating at $jump intervals

foreach ($erMin .. $erMax)
{
	push @jumps, $_
		if $_ % $jump == 0;

}

#Avoiding problems with a decimal parameter
push @jumps, $jump
	if(scalar @jumps == 0);
push @jumps, $erMax
	if ($erMax > 0 and $jumps[-1] != $erMax);

OUTTER: foreach my $i (@jumps){
	INNER: foreach my $j (1 .. $rep){
		my $seed = int(rand()*1000);
		my $error = $i/100;
		@command = ("./ECA_exec $time $N $length $bandwidth $batch $ECA $stickiness $fairShare $error $drift $EDCA $maxAggregation $seed"); 
		print ("\n\n****Error #$i of $erMax ($?).\n");
		print ("****Iteration #$j of $rep.\n");
		print ("**** @command\n");
		system(@command);
		(print ("\n\n********Execution failed\n\tQuitting iterations\n") and last OUTTER) if ($? != 0);
	}
}


#Calling the parser
my $parserFile = 'processVarError.pl';
# my $parseSlots = 'analyseSlots.pl';
my $dataFile = 'Results/output.txt';
# my $slotsFile = 'Results/slotsInTime.txt';
my @parseCommand = ("./$parserFile $dataFile");
system(@parseCommand);
(print ("\n\n********Processing failed\n") and last OUTTER) if ($? != 0);

# @parseCommand = ("perl $parseSlots $slotsFile");
# system(@parseCommand);
# (print ("\n\n********Processing failed\n") and last OUTTER) if ($? != 0);