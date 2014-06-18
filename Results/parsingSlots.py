#!/usr/lib/python

import csv
from decimal import Decimal
import sys, getopt #to manage arguments passed from CLI
import math

def main(argv):
	inputfile = ''
	outputfile = ''
	rows = 0 #augmenting the rows
	
	slotStatistics = {}
	numberOfHosts = []
	
	#Input file-related columns	
	nodes = 0;
	time = 1;
	slotCount = 2;
	collisionSlots = 3;
	sxSlots = 4;
	emptySlots = 5;

#------Managing the input file------#

	opts, args = getopt.getopt(argv, "hi:o:", ["ifile=", "ofile="]) #h and i are two of the options accepted (help and input)
								#semicolon next to i means that it accepts an argument (as well as with o)
	for opt,arg in opts:
		if opt == '-h':
			print 'parsingSlots.py -i <inputFile>'
			sys.exit()
		elif opt in ("-i", "--ifile"):
			inputfile = arg
		elif opt in ("-o", "--ofile"):
			outputfile = arg

	print 'Processing: ', inputfile
	print 'Outputting into: ', outputfile

#------Processing------#

	with open(inputfile, 'r') as file: #opens the file
		capture = list(csv.reader(file, delimiter=' ')) #stores the content of the csv as a list
		for lines in capture:
			numNodes = int(capture[rows][nodes])
			slots = int(capture[rows][slotCount])
			if numNodes not in slotStatistics:
				###################################################
				###########Capturing statistics per slots##########
				#slotsStatistics: {numNodes[[slots:[[0. time], [1. collision], [2. successful], [3. empty]]]]}
				###################################################
				slotStatistics[numNodes] = {}
				numberOfHosts.append(numNodes)
			if slots not in slotStatistics[numNodes]:
				slotStatistics[numNodes][slots] = [[],[],[],[]]
			
			slotStatistics[numNodes][slots][0].append(capture[rows][time])
			slotStatistics[numNodes][slots][1].append(capture[rows][collisionSlots])
			slotStatistics[numNodes][slots][2].append(capture[rows][sxSlots])			
			slotStatistics[numNodes][slots][3].append(capture[rows][emptySlots])
			
			rows += 1
	file.close()

	exportGNUPlot(slotStatistics, numberOfHosts, outputfile)

	###############
	#Screen output#
	###############	
	print "\n"
	for h in numberOfHosts:
		print "###For host: ", h
		for s in slotStatistics[h]:
			print "######Slot: ", s
			print "#########Average Pc:", average(slotStatistics[h][s][1])
			print "#########Average Tau: ", average(slotStatistics[h][s][2])
		print "\n"
	
######################
#####Functions########
######################
def average(listOfValues):	
	if len(listOfValues) > 0:
		numerator = 0
		counter = 0
		for item in listOfValues:
			numerator = numerator + float(item)
			counter = counter + 1
		return numerator/counter
	else:
		return 0
		
def std(listOfValues):
	if len(listOfValues) > 1:
		numerator = 0
		counter = 0
		mean = average(listOfValues)
		for item in listOfValues:
			numerator = numerator + (math.fabs(float(item) - mean)**(2))
			counter = counter + 1
		return (numerator/(counter - 1))**(0.5)
	else:
		return 0
		
def exportGNUPlot(dictionary, listOfHosts, file):
	with open(file, 'w') as output:
		output.write("#0. numberOfHosts, 1. slotCount, 2. SxSlots, 3. stdSxSlots, 4. ColSlots, 5. stdColSlots, 6. EmptySlots, 7. stdEmptySlots" + '\n')
		for h in listOfHosts:
			for s in dictionary[h]:
				output.write(str(h) + ' ' + str(s) + ' ' + str(average(dictionary[h][s][2])) + ' ' + str(std(dictionary[h][s][2])) + ' ' + str(average(dictionary[h][s][1])) + ' ' + str(std(dictionary[h][s][1])) + ' ' + str(average(dictionary[h][s][3])) + ' ' + str(std(dictionary[h][s][3])) + '\n')
	output.close()
		
		
if __name__ == "__main__":
	main(sys.argv[1:])
