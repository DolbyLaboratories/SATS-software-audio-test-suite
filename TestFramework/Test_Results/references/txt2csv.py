import csv
import sys
import glob
import os

def txt2Csv(filename):
    txtFile = []
    with open(filename,'rb') as infile:
	for line in infile:
	    if line == '\r\n' or line == '\n':
		print('Found Empty line and removed it !!')
		continue
	    txtFile.append(line.rstrip('\r\n').split('\t'))
    
    with open('../referencesCSV/' + filename + '.csv','wb') as outfile:
	writer = csv.writer(outfile)
	writer.writerows(txtFile)
	del writer
	print("Done!")
    return
  
filepath = '*'
files = glob.glob(filepath)
for f in files:
    txt2Csv(f)
os.remove('../referencesCSV/txt2csv.py.csv')