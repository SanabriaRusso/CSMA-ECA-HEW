#!/bin/perl
use warnings;
use strict;


my $outputFile = 'plotFile.dat';

open(my $fh, '-|', 'for FILE in $( ls ); do cat $FILE; done')
	or die "Could not open files ($!)\n";

open(my $wh, '>', $outputFile)
	or die "Could not create write file ($!)\n";

while(my $row = <$fh>)
{
	chomp $row;
	if($row =~ /^\d/)
	{	
		print "$row\n";
		print $wh ("$row\n");
	}
}
