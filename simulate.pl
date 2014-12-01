#!/usr/bin/perl

use warnings;
use strict;

die ("Getting out. You need some parameters, i.e.: --help\n") if (not exists $ARGV[0]);

chomp($ARGV[0]);

my %help = (
				help		=> 1,
				'-h'		=> 1,
				'--help'	=> 1
			);

die ("******Help\n", "ARGV. field:\n", "0. Repetitions 1. Time 2. Nmax 3. Nmin 4. Jumps 5. Bandwidth 6. ECA\n") 
	if (exists $help{$ARGV[0]});

my $rep = $ARGV[0];
my $time = $ARGV[1];
my $Nmax = $ARGV[2];
my $Nmin = $ARGV[3];
my $jump = $ARGV[4];
my $bandwidth = $ARGV[5];
my $ECA = $ARGV[6];
my $length = 1024;
my $batch = 1;
my $errors = 0;
my $drift = 0;
my $EDCA = 0;
my $maxAggregation = 0;

my $hysteresis = 0;
my $fairShare = 0;

print ("Going to simulate:\n");
if ($Nmax == $Nmin){
	print ("\t$rep repetitions of $time seconds, ", $Nmin, " stations, $bandwidth Mbps and ECA $ECA.\n\n\n");	
}else{
	print ("\t$rep repetitions jumping $jump of $time seconds, ", ($Nmax - $Nmin), " stations, $bandwidth Mbps and ECA $ECA.\n\n\n");
}

if ($ECA == 1){
	$hysteresis = 1;
	$fairShare = 1;
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
	if @jumps[-1] != $Nmax;


OUTTER: foreach my $i (@jumps){
	INNER: foreach my $j (1 .. $rep){
		my $seed = int(rand()*1000);
		@command = ("./ECA_exec $time $i $length $bandwidth $batch $ECA $hysteresis $fairShare $errors $drift $EDCA $maxAggregation $seed"); 
		print ("\n\n****Node #$i of $Nmax ($?).\n");
		print ("****Iteration #$j of $rep.\n");
		print ("**** @command\n");
		system(@command);
		(print ("\n\n********Execution failed\n\tQuitting iterations\n") and last OUTTER) if ($? != 0);
	}
}


#Calling the parser
my $parserFile = 'process.pl';
my $dataFile = 'Results/output.txt';
my @parseCommand = ("perl $parserFile $dataFile");
system(@parseCommand);
(print ("\n\n********Processing failed\n") and last OUTTER) if ($? != 0);