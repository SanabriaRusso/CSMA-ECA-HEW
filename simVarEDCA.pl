#!/usr/local/bin/perl
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

die ("******Help\n", "ARGV. field:\n", "0. Repetitions 1. Time 2. Nmax 3. Bandwidth 4. Channel errors 5. EDCA min (%) 6. EDCA max (%) 7. Jumps 8. ECA Code 9. numACs 10. LENGTH(Bytes)\n") 
	if (exists $help{$ARGV[0]});

my $rep = $ARGV[0];
my $time = $ARGV[1];
my $Nmax = $ARGV[2];
my $bandwidth = $ARGV[3];
my $errors = $ARGV[4];
my $EDCA_min = $ARGV[5];
my $EDCA_max = $ARGV[6];
my $jump = $ARGV[7];
my $ECA = $ARGV[8];
my $numACs = $ARGV[9];
my $length = $ARGV[10];
my $batch = 1;
my $drift = 0;
my $maxAggregation = 0;

my $stickiness = 0;
my $fairShare = 0;

print ("Going to simulate:\n");
if ($EDCA_max == $EDCA_min){
	print ("\t$rep repetitions of $time seconds, ", $Nmax, " stations, $bandwidth Mbps, errors $errors and ECA $ECA with $numACs ACs.\n\n\n");	
}else{
	print ("\t$rep repetitions jumping $jump from $EDCA_min to $EDCA_max of $time seconds, $Nmax stations, $bandwidth Mbps, errors $errors and ECA $ECA with $numACs ACs.\n\n\n");
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
		$stickiness = 1;
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
if( $EDCA_max - $EDCA_min > 0 ){
	foreach ($EDCA_min .. $EDCA_max)
	{
		push @jumps, $_
			if $_ % $jump == 0;

	}
	push @jumps, $EDCA_max
		if $jumps[-1] != $EDCA_max;
}else{
	push @jumps, $EDCA_max;
}

#Normalizing the jump of percentages of EDCA nodes
for my $i (0 .. $#jumps){
	$jumps[$i] = $jumps[$i]/100;
}


OUTTER: foreach my $i (@jumps){
	INNER: foreach my $j (1 .. $rep){
		my $seed = int(rand()*1000);
		@command = ("./ECA_exec $time $Nmax $length $bandwidth $batch $ECA $stickiness $fairShare $errors $drift $i $maxAggregation $numACs $seed"); 
		print ("\n\n****EDCA share ", $i*100, "% of ", $EDCA_max, "% ($?).\n");
		print ("****Iteration #$j of $rep.\n");
		print ("**** @command\n");
		system(@command);
		(print ("\n\n********Execution failed\n\tQuitting iterations\n") and last OUTTER) if ($? != 0);
	}
}


#Calling the parser
my $parserFile = 'processVarEDCA.pl';
my $dataFile = 'Results/output.txt';
my @parseCommand = ("./$parserFile $dataFile");
system(@parseCommand);
(print ("\n\n********Processing failed\n") and last OUTTER) if ($? != 0);

#my $simulation = "$bandwidth-$ECA-$stickiness-$fairShare-$errors";
#my @mail = ("./sendMail $simulation");
#system(@mail);



