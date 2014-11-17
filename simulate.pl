#!/usr/bin/perl

use warnings;
use strict;


chomp($ARGV[0]);
( ($ARGV[0] eq '-h') or ($ARGV[0] eq '--help') or ($ARGV[0] eq 'help') or ($ARGV[0] eq 'h') ) and goto HELP;


my $rep = $ARGV[0];
my $time = $ARGV[1];
my $Nmax = $ARGV[2];
my $Nmin = $ARGV[3];
my $bandwidth = $ARGV[4];
my $ECA = $ARGV[5];
my $length = 1024;
my $batch = 1;
my $errors = 0;
my $drift = 0;
my $EDCA = 0;
my $maxAggregation = 0;

print ("Going to simulate:\n");
if ($Nmax == $Nmin){
	print ("\t$rep repetitions of $time seconds, ", $Nmin, " stations, $bandwidth Mbps and ECA $ECA.\n\n\n");	
}else{
	print ("\t$rep repetitions of $time seconds, ", ($Nmax - $Nmin), " stations, $bandwidth Mbps and ECA $ECA.\n\n\n");
}

my $hysteresis = 0;
my $fairShare = 0;

if ($ECA == 1){
	$hysteresis = 1;
	$fairShare = 1;
}

my $compile = './build_local';
my @command;

system($compile);
if ($? == -1){
	print "Command failed\n";
}else{
	print ("\n\nCompilation compleated (", $?, ")\n\n");
}


foreach my $i ($Nmin .. $Nmax){
	foreach my $j (1 .. $rep){
		my $seed = int(rand()*1000);
		@command = ("./ECA_exec $time $i $length $bandwidth $batch $ECA $hysteresis $fairShare $errors $drift $EDCA $maxAggregation $seed"); 
		print @command, "\n";
		system(@command);
		($? == -1) and 	print ("\n\n******Execution failed\n\tQuitting iterations\n") and goto EXITNOW;
	}
}


EXITNOW:
(print "\tExecution finished ($?)\n") and exit();

HELP:
print "******Help\n";
print "ARGV. field:\n";
print "0. Repetitions 1. Time 2. Nmax 3. Nmin 4. Bandwidth 5. ECA\n";
exit()




