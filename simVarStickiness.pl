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

die ("******Help\n", "ARGV. field:\n", "\t0. Repetitions\n \t1. Time\n \t2. Nodes\n \t3. Bandwidth\n \t4. Channel errors\n \t5. Jumps\n \t6. Stickiness Min\n \t7. Stickiness Max\n") 
	if (exists $help{$ARGV[0]});

my $rep = $ARGV[0];
my $time = $ARGV[1];
my $N = $ARGV[2];
my $bandwidth = $ARGV[3];
my $errors = $ARGV[4];
my $jump = $ARGV[5];
my $stickMin = $ARGV[6];
my $stickMax = $ARGV[7];
my $length = 1024;
my $batch = 1;
my $drift = 0;
my $ECA = 1;
my $EDCA = 0;
my $maxAggregation = 0;
my $stickiness = 0;
my $fairShare = 1;

print ("Going to simulate:\n");
if ($stickMax == $stickMin){
	print ("\t$rep repetitions of $time seconds, $N stations, $bandwidth Mbps, errors $errors and ECA with stickiness $stickMax.\n\n\n");	
}else{
	print ("\t$rep repetitions jumping $jump of $time seconds, $errors channel errors, $N stations, $bandwidth Mbps and ECA ", ($stickMax - $stickMin), " levels of stickiness.\n\n\n");
}

my $compile = './build_local';
my @command;
my @jumps;

system($compile);
die "Command failed\n" if ($? != 0);

#Simulating at $jump intervals

foreach ($stickMin .. $stickMax)
{
	push @jumps, $_
		if $_ % $jump == 0;

}
push @jumps, $stickMax
	if $jumps[-1] != $stickMax;


OUTTER: foreach my $i (@jumps){
	INNER: foreach my $j (1 .. $rep){
		my $seed = int(rand()*1000);
		@command = ("./ECA_exec $time $N $length $bandwidth $batch $ECA $i $fairShare $errors $drift $EDCA $maxAggregation $seed"); 
		print ("\n\n****Stickiness #$i of $stickMax ($?).\n");
		print ("****Iteration #$j of $rep.\n");
		print ("**** @command\n");
		system(@command);
		(print ("\n\n********Execution failed\n\tQuitting iterations\n") and last OUTTER) if ($? != 0);
	}
}


#Calling the parser
my $parserFile = 'processVarStickiness.pl';
# my $parseSlots = 'analyseSlots.pl';
my $dataFile = 'Results/output.txt';
# my $slotsFile = 'Results/slotsInTime.txt';
my @parseCommand = ("./$parserFile $dataFile");
system(@parseCommand);
(print ("\n\n********Processing failed\n") and last OUTTER) if ($? != 0);

# @parseCommand = ("perl $parseSlots $slotsFile");
# system(@parseCommand);
# (print ("\n\n********Processing failed\n") and last OUTTER) if ($? != 0);