##########################################################################################################
# Copyright (c) 2018, Dolby Laboratories Inc.
# All rights reserved.

# Redistribution and use in source and binary forms, with or without modification, are permitted
# provided that the following conditions are met:

# 1. Redistributions of source code must retain the above copyright notice, this list of conditions
#    and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions
#    and the following disclaimer in the documentation and/or other materials provided with the distribution.
# 3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or
#    promote products derived from this software without specific prior written permission.

# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
# WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
# PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
# ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
# OF THE POSSIBILITY OF SUCH DAMAGE.
#############################################################################################################

############################################################################
#	File:	testScript.py

#	History:
#		13/06/18		Created		Author: Dolby		Version: 1.0
###########################################################################

import sys
import os
import glob
import csv
import subprocess
import time

# GlobalVariable
flag = 0
check = 0
check1 = 0
binpath = ''
binname = ''
binplt_tmp = ''
binplt = ''
binver = ''
stdiover = ''
bincheck = ''
lib = 'kiss'


def checkPyVer():
	if sys.version_info.major > 2:
		py_ver = str(sys.version_info.major)+'.'+str(sys.version_info.minor)
		print('Python version '+py_ver+' found')
		print('Please use version 2.7')
		print('Closing down ..')
		sys.exit()
	return
	
# Exit Prompt
def exitPrompt():
	ans = raw_input("Do you wish to continue? [y]es or [n]o: ")
	if ans == "n":
		print("Closing down ..")
		print("")
		sys.exit()
	elif ans == "y":
		options()
	else:
		print("invalid input")
	return


# Deleting files from a directory
def clearFiles(filepath):
	files = glob.glob(filepath)
	for f in files:
		os.remove(f)
	return


#Choice of Platform
def choosePlt():
	global binplt
	global binplt_tmp
	print("")
	print("Choose from the following Platforms (Note: Binary to be tested must be built for this platform) : \n")
	print("[1]linux_amd64_gnu\n[2]linux_x86_gnu\n[3]windows_x86_gnu\n[4]windows_amd64_msvs\n[5]windows_x86_msvs\n[6]exit\n")
	print("################################\n")
	choice = (raw_input("Your choice: "))
	if choice == '1':
		binplt = 'linux_amd64_gnu'
	elif choice == '2':
		binplt = 'linux_x86_gnu'
	elif choice == '3':
		binplt = 'windows_x86_gnu'
	elif choice == '4':
		binplt = 'windows_amd64_msvs'
		studioVer()
	elif choice == '5':
		binplt = 'windows_x86_msvs'
		studioVer()
	elif choice == '6':
		print("Closing down ..")
		print("")
		sys.exit()
	else:
		print("invalid input")
		exitPrompt()
	binplt_tmp = binplt
	return

def chooseLib():
	global lib
	global binplt
	lib = 'kiss'
	if  (binplt != 'windows_x86_gnu'):
		print("")
		print("Choose a library for FFT calculations (Note: Intel library files needs to be downloaded separately and added to the folder intel_mkl , Refer readme for folder structure ) : \n")
		print("[1]KISS_FFT\n[2]Intel_MKL_FFT\n[3]exit\n")
		print("################################\n")
		choice = (raw_input("Your choice: "))
		if choice == '1':
			lib = 'kiss'
		elif choice == '2':
			lib = 'intel'
		elif choice == '3':
			print("Closing down ..")
			print("")
			sys.exit()
		else:
			print("invalid input")
			exitPrompt()
	return

# Choice of Version
def chooseVer():  
	global binver
	global binplt
	global binplt_tmp
	binplt = binplt_tmp
	if  binplt == 'windows_x86_gnu':
		print("")
		print("Default usage of KISS library for FFT calculations for the chosen platform")
	print("")
	print("Choose the version of the binary to be tested : \n")
	print("[1]debug\n[2]release\n[3]exit\n")
	print("################################\n")
	choice = (raw_input("Your choice: "))
	print("")
	print(binplt)
	if choice == '1':
		if (binplt == 'windows_amd64_msvs' or binplt == 'windows_x86_msvs'):
			binplt = os.path.join(binplt,'debug',stdiover)
		else:
			binver = '_debug'
	elif choice == '2':
		if (binplt == 'windows_amd64_msvs' or binplt == 'windows_x86_msvs'):
			binplt = os.path.join(binplt,'release',stdiover)
		else:
			binver = '_release'
	elif choice == '3':
		print("Closing down ..")
		print("")
		sys.exit()
	else:
		print("invalid input")
		exitPrompt()
	return

def studioVer():
	global stdiover
	print("")
	print("Choose the Visual Studio version : \n")
	print("[1]2015\n[2]2017\n[3]exit\n")
	print("################################\n")
	choice = (raw_input("Your choice: "))
	if choice == '1':
		stdiover = 'VS2015'
	elif choice =='2':
		stdiover = 'VS2017'
	elif choice == '3':
		print("Closing down ..")
		print("")
		sys.exit()
	else:
		print("invalid input")
		exitPrompt()
	return

# Choice of Binaries
def chooseBin():
	global lib
	print("")
	print("Choose from the following applications: \n")
	#print("[1]amp_vs_time\n[2]bandwidth\n[3]dyn_rng\n[4]freq_resp\n[5]noise_mod\n[6]pwr_vs_time\n[7]spectrogram\n[8]spectrum_avg\n[9]spectrum_NFFT\n[10]thd_vs_freq\n[11]thd_vs_level\n")
	print("[1] All (All binaries for the given platform will be tested)\n[2]amp_vs_time\n[3]dyn_rng\n[4]freq_resp\n[5]mult_freq_resp\n[6]noise_mod\n[7]pwr_vs_time\n[8]spectrum_avg\n[9]spectrum_NFFT\n[10]thd_vs_freq\n[11]thd_vs_level\n[12]exit\n")
	print("################################\n")
	choice = (raw_input("Your choice: "))
	if choice == '1':
		testall()
	elif choice == '2':
		testbin('amp_vs_time',amp_vs_time,1,'')
	elif choice == '3':
		testbin('dyn_rng',dyn_rng,1,'')
	elif choice == '4':
		testbin('freq_resp',freq_resp,4,'')
	elif choice == '5':
		chooseLib()
		testbin('mult_freq_resp',mult_freq_resp,2,lib)
	elif choice == '6':
		testbin('noise_mod',noise_mod,1,'')
	elif choice == '7':
		testbin('pwr_vs_time',pwr_vs_time,1,'')
	elif choice == '8':
		chooseLib()
		testbin('spectrum_avg',spectrum_avg,1,lib)
	elif choice == '9':
		chooseLib()
		testbin('spectrum_NFFT',spectrum_nfft,3,lib)
	elif choice == '10':
		testbin('thd_vs_freq',thd_vs_freq,1,'')
	elif choice == '11':
		testbin('thd_vs_level',thd_vs_level,1,'')
	elif choice == '12':
		print("Closing down ..")
		print("")
		sys.exit()
	else:
		print("invalid input")
	exitPrompt()
	return

# Check if binary exists
def binCheck(binpath,x):
	if x == 1:
		cmd = binpath+ ' -to Test_Results/test_bin -c a -i Test_Signals/vanilla.wav'
	elif x == 2:
		cmd = binpath+ ' -to Test_Results/test_bin -c a -f Test_Signals/dv_30_processed_file.txt -i Test_Signals/dv_30_processed_file.wav'
	elif x == 3:
		cmd = binpath+ ' -to Test_Results/test_bin -c a -n 512 -i Test_Signals/vanilla.wav'
	else:
		cmd = binpath+ ' -to Test_Results/test_bin -c a -i Test_Signals/2_test_frqstp_32000.wav'
	try:
		subprocess.check_output(cmd,shell=True)
	except subprocess.CalledProcessError as e:
		print(e.output)
		print('Binary not found!')
		exitPrompt()
	return

#test Binary
def testbin(name,func,check,l):
	global binpath
	global binname
	global binver
	global binplt
	global bincheck
	binname = name
	if l == 'intel':
		binpath = os.path.join('..','make',binname+'_intel',binplt,binname+'_intel'+binver)
	else:
		binpath = os.path.join('..','make',binname,binplt,binname+binver)
	binCheck(binpath,check)
	return func()

#Test all binaries
def testall():
	global binver
	global binplt
	print('All binaries for the platform: '+binplt+ ' and for the version: '+binver+ ' will be tested.')
	print('The binaries will be tested successively in the order displayed in the choice above.')
	print('When there is a failure in any test case for a binary, the complete test report of that binary will be displayed along with the failed test IDs and the testing will halt. That is, the successive binaries will not be tested unless the issue is fixed.')
	print('Note that you can always choose an individual binary/executable to be tested if you want to skip some failed test cases in a particular binary.')
	global lib
	chooseLib()
	start = time.time()
	testbin('amp_vs_time',amp_vs_time,1,'')
	testbin('dyn_rng',dyn_rng,1,'')
	testbin('freq_resp',freq_resp,1,'')
	testbin('mult_freq_resp',mult_freq_resp,2,lib)
	testbin('noise_mod',noise_mod,1,'')
	testbin('pwr_vs_time',pwr_vs_time,1,'')
	testbin('spectrum_avg',spectrum_avg,1,lib)
	testbin('spectrum_NFFT',spectrum_nfft,3,lib)
	testbin('thd_vs_freq',thd_vs_freq,1,'')
	testbin('thd_vs_level',thd_vs_level,1,'')
	end = time.time()
	print('Time Elapsed for the Test All case :')
	print(end - start)
	return
   
# Print statements for comparison
def printCompare():
	print("")
	print("Comparing your results with the reference file ...")
	print("Computing difference between the result and reference ...")
	print("Looking for differences greater than 0.01 dB ...")
	print("")
	return


# Print statements for test call    
def printCall():
	print("")
	print("Your results are saved with the corresponding testID name (txt file) in Test_Results folder")
	print("A copy of the results is available in csv format in Test_Results/csvFiles folder  ...")
	return
  

#Print statements for result
def printResult():
	if check == 1:
		print("#############################")
		print("Passed for other Test IDs")
		print("#############################")
		print("")
		sys.exit()
	else:
		print("")
		print("#############################")
		print("Passed for all Test IDs ")
		print("#############################")
		print("")
	return

  
#Call resultCompare for different test IDs
def callCompare(ID_name,fro,to):
	for x in range(fro,to):
		y = str("%02d" % x)
		resultCompare(ID_name+'_'+y)
	return

#Call resultCompare for different test IDs for spectrum_NFFT test case 
def callCompareNfft(fro,to):
	for x in range(fro,to):
		y = str("%02d" % x)
		resultCompare('spectrum_NFFT_'+y+'_512')
		resultCompare('spectrum_NFFT_'+y+'_1024')
		resultCompare('spectrum_NFFT_'+y+'_4096')
		resultCompare('spectrum_NFFT_'+y+'_8192')
		resultCompare('spectrum_NFFT_'+y+'_10240')
	return

#Function to write txt file to csv file
def txt2Csv(filename):
	#txtFile = [ line.rstrip('\n').split('\t') for line in open('Test_Results/' + filename)]
	txtFile = []
	for line in open('Test_Results/' + filename):
		if line == '\n':
			print('Found empty line during conversion to csv and removed it ')
			continue
		txtFile.append(line.rstrip('\n').split('\t'))
	writer = csv.writer(file('Test_Results/csvFiles/' + filename + '.csv','wb'))
	writer.writerows(txtFile)
	del writer
	return


# Function to compare current result with the reference
def resultCompare(filename):  
	global flag
	global check
	global check1
	with open('Test_Results/csvFiles/' +filename+ '.csv','r') as book1:
	#with open('Test_Results/csvFiles/amp_vs_time_01.csv','r') as book1:
		with open('Test_Results/referencesCSV/' +filename+ '.res.csv','r') as book2:
			reader1 = csv.reader(book1, delimiter=',')
			reader2 = csv.reader(book2, delimiter=',')
			both = []
			for row1, row2 in zip(reader1, reader2):
				row2.append(row1[-1])
				row2.append(float(row1[1]) - float(row2[1]))
				both.append(row2)
			with open('Test_Results/Error_Difference/' +filename+ '_diff.csv','w') as output:
				writer = csv.writer(output, delimiter=',')
				writer.writerow(['x-axis', 'Reference', 'Result','Difference'])
				writer.writerows(both)
			row1_count = sum(1 for row in reader1)
			row2_count = sum(1 for row in reader2)
			#print(row1_count)
			#print(row2_count)
			if row1_count != row2_count:
				check = 1
				check1 = 1
	with open('Test_Results/Error_Difference/' +filename+ '_diff.csv','r') as book3:
		reader3 = csv.reader(book3, delimiter=',')
		reader3.next()
		for row3 in reader3:
			if round(abs(float(row3[3])),3) > 0.01:
				flag = 1
				check = 1
				check1 = 0
				continue
			else:
				pass
		if flag == 1:
			print("#############################")
			print("Failed for Test ID " +filename)
			print("Look at the " +filename+ "_diff.csv file in /Test_Results/Error_Difference for computed differences")
			print("#############################")
			print("")
			flag = 0
		elif check1 == 1:
			print("#############################")
			print("Failed for Test ID " +filename)
			print("Some or all channels missing from the result")
			print("#############################")
			print("")
			check1 = 0
			book3.close()
			os.remove('Test_Results/Error_Difference/' +filename+ '_diff.csv')
		else:
			book3.close()
			os.remove('Test_Results/Error_Difference/' +filename+ '_diff.csv')
	return


# Function to call test cases
def callTest(testID,testsignal,channel):
	global binpath
	print("")
	print("Test ID : " +testID)
	if channel == 1:
		print("Calculating results for '" +testsignal+ "' signal for single channel '0' ...")
		cmd = binpath+ ' -to Test_Results/'+testID+ ' -c 0 -i Test_Signals/' +testsignal
	elif channel == 2:
		print("Calculating results for '" +testsignal+ "' signal for channels '0' and '1' ...")
		cmd = binpath+ ' -to Test_Results/'+testID+ ' -c a -i Test_Signals/' +testsignal
	else:
		print("Calculating results for '" +testsignal+ "' signal for all channels")
		cmd = binpath+ ' -to Test_Results/'+testID+ ' -c a -i Test_Signals/' +testsignal
	print(cmd)
	try:
		#print(cmd)
		#os.system(cmd)
		subprocess.call(cmd,shell=True)
		txt2Csv(testID)
	except Exception:
		if testID == 'dyn_rng_12' or testID == 'dyn_rng_13' or testID == 'freq_resp_18' or testID == 'freq_resp_19' or testID == 'noise_mod_06' or testID == 'noise_mod_07' \
			 or testID == 'spectrum_avg_07' or testID == 'spectrum_avg_08' or testID == 'thd_vs_freq_18' or testID == 'thd_vs_freq_19' or testID == 'thd_vs_level_09' \
			 or testID == 'thd_vs_level_10':
			print("**************")
			print("Binary throws error for invalid frequency/bit rate")
			print("Passed for testID " +testID)
			print("**************")
	return

#Function to call test cases for amp_vs_time with xmin and xmax options
def callxTest(testID,testsignal,xmin,xmax):
	global binpath
	xmin = str(xmin)
	xmax = str(xmax)
	print("")
	print("Test ID : " +testID)
	print("Calculating results for '" +testsignal+ "' signal for all channels")
	cmd = binpath+ ' -to Test_Results/'+testID+ ' -c a -xmin ' +xmin+ ' -xmax ' +xmax+ ' -i Test_Signals/' +testsignal
	print(cmd)
	#os.system(cmd)
	subprocess.call(cmd,shell=True)
	txt2Csv(testID)
	return

#Function to call test cases for spectrum_nfft
def callNfftTest(testID,testsignal,channel,fft,pwr):
	global binpath
	text = ''
	com = ''
	if pwr == 'n':
		pass
	else:
		pwr = str(pwr)
		text = ' ***with pre-defined dB level ' +pwr+ 'dB to which very low power values will be clipped***'
		com = ' -powermin ' +pwr
	print("")
	print("Test ID : " +testID)
	fft = str(fft)
	if channel == 1:
		print("Calculating results for '" +testsignal+ "' signal for single channel '0' and fft size " +fft+ text+" ...")
		cmd = binpath+ ' -to Test_Results/'+testID+ ' -c 0 -s -n '+fft+ com+ ' -i Test_Signals/' +testsignal
		#cmd = '../Test/itaf_based/dut_binaries/linux32/spectrum_NFFT_dut -to Test_Results/'+testID+ ' -c 0 -n '+fft+ com+ ' -i Test_Signals/' +testsignal
	elif channel == 2:
		print("Calculating results for '" +testsignal+ "' signal for channels '0' and '1' and fft size " +fft+ text+" ...")
		cmd = binpath+ ' -to Test_Results/'+testID+ ' -c a -s -n '+fft+ com+ ' -i Test_Signals/' +testsignal
		#cmd = '../Test/itaf_based/dut_binaries/linux32/spectrum_NFFT_dut -to Test_Results/'+testID+ ' -c a -n '+fft+ com+ ' -i Test_Signals/' +testsignal
	else:
		print("Calculating results for '" +testsignal+ "' signal for all channels and fft size " +fft+ text+" ...")
		cmd = binpath+ ' -to Test_Results/'+testID+ ' -c a -s -n '+fft+ com+ ' -i Test_Signals/' +testsignal
		#cmd = '../Test/itaf_based/dut_binaries/linux32/spectrum_NFFT_dut -to Test_Results/'+testID+ ' -c a -n '+fft+ com+ ' -i Test_Signals/' +testsignal
	#os.system(cmd)
	print(cmd)
	subprocess.call(cmd,shell=True)
	txt2Csv(testID)
	return

#Function to call special test cases for spectrum_nfft whcih include -z and window options
def callNfftSplTest(testID,testsignal,fft,value,option):
	global binpath
	text = ''
	com = ''
	samples = 0
	if option == 'z':
		navg = str(value)
		com = ' -z ' +navg
		samples = (fft/2)*(int(navg)+1)
		samples = str(samples)
		text = ' ***with only pre-defined number of blocks: '+navg+ ' to be computed ***\n Total number of samples considered: '+samples
	else:
		window = str(value)
		if window == '1':
			text = ' ***with pre-defined window type : Bartlett Window ***'
		elif window == '2':
			text = ' ***with pre-defined window type : Bartlett Hann Window ***'
		elif window == '4':
			text = ' ***with pre-defined window type : Rectangular Window ***'
		elif window == '5':
			text = ' ***with pre-defined window type : Triangular Window ***'
		else:
			text = ' ***with pre-defined window type : Hann Window ***'
		com = ' -window ' +window
	print("")
	print("Test ID : " +testID)
	fft = str(fft)
	print("Calculating results for '" +testsignal+ "' signal for single channel '0' and fft size " +fft+ text+" ...")
	cmd = binpath+ ' -to Test_Results/'+testID+ ' -c 0 -s -n '+fft+ com+ ' -i Test_Signals/' +testsignal
	#os.system(cmd)
	print(cmd)
	subprocess.call(cmd,shell=True)
	txt2Csv(testID)
	return

# Function to call test cases for pwr_vs_time
def callPowerTest(testID,testsignal,channel,opt,value):
	global binpath
	blk = ''
	unit = ''
	if opt == 'bs':
		blk = 'blksz_s'
		unit = 'samples'
	else:
		blk = 'blksz_t'
		unit = 'ms'
	print("")
	print("Test ID : " +testID)
	value = str(value)
	if channel == 1:
		print("Calculating results for '" +testsignal+ "' signal for single channel '0' with block size of " +value+ " "+unit+ "...")
		cmd = binpath+ ' -to Test_Results/'+testID+ ' -c 0 -s -' +blk+ ' ' +value+ ' -i Test_Signals/' +testsignal
	elif channel == 2:
		print("Calculating results for '" +testsignal+ "' signal for channels '0' and '1' with block size of" +value+ " " +unit+ "...")
		cmd = binpath+ ' -to Test_Results/'+testID+ ' -c a -s -' +blk+ ' ' +value+ ' -i Test_Signals/' +testsignal
	else:
		print("Calculating results for '" +testsignal+ "' signal for all channels with block size of" +value+ " " +unit+ "...")
		cmd = binpath+ ' -to Test_Results/'+testID+ ' -c a -s -' +blk+ ' ' +value+ ' -i Test_Signals/' +testsignal
	#os.system(cmd)
	print(cmd)
	subprocess.call(cmd,shell=True)
	txt2Csv(testID)
	return

#Function to call test cases for mult_freq_resp
def callMultTest(testID,testsignal):
	global binpath
	print("")
	print("Test ID : " +testID)
	print("Calculating results for '" +testsignal+ ".wav' signal for channels '0' and '1' ...")
	cmd = binpath+ ' -to Test_Results/'+testID+ ' -c a -f Test_Signals/' +testsignal+ '.txt -i Test_Signals/' +testsignal+ '.wav'
	#os.system(cmd)
	print(cmd)
	subprocess.call(cmd,shell=True)
	txt2Csv(testID)
	return

# Function to call test cases without stripping lead silence
def callnStripTest(testID,testsignal):
	global binpath
	print("")
	print("Test ID : " +testID)
	print("Calculating results for '" +testsignal+ "' signal ***without stripping lead silence***")
	cmd = binpath+ ' -to Test_Results/'+testID+ ' -c 0 -s -i Test_Signals/' +testsignal
	#os.system(cmd)
	print(cmd)
	subprocess.call(cmd,shell=True)
	txt2Csv(testID)
	return
 
# Function to call test cases with pre-defined dB level for stripping lead silence
def callLevelTest(testID,testsignal,opt,level,blksz):
	global binpath
	print("")
	level = str(level)
	blksz = str(blksz)
	print("Test ID : " +testID)
	if opt == 'pm':
		powermin = str(level)
		print("Calculating results for '" +testsignal+ "' signal ***with pre-defined dB level " +powermin+ "dB to which very low power values will be clipped***")
		cmd = binpath+ ' -to Test_Results/'+testID+ ' -c 0 -powermin '+powermin+' -i Test_Signals/' +testsignal
	elif opt == 'db':
		dblevel = str(level)
		print("Calculating results for '" +testsignal+ "' signal ***with pre-defined dB level " +dblevel+ "dB for stripping lead silence***")
		cmd = binpath+ ' -to Test_Results/'+testID+ ' -c 0 -thr_db '+dblevel+' -i Test_Signals/' +testsignal
	elif opt == 'pmb':
		powermin = str(level)
		value = str(blksz)
		print("Calculating results for '" +testsignal+ "' signal with block size of" +value+ " samples  ***with pre-defined dB level " +powermin+ "dB to which very low power values will be clipped***")
		cmd = binpath+ ' -to Test_Results/'+testID+ ' -c a -s -blksz_s '  +value+ ' -powermin '+powermin+' -i Test_Signals/' +testsignal
	elif opt == 'dbb':
		dblevel = str(level)
		value = str(blksz)
		print("Calculating results for '" +testsignal+ "' signal with block size of" +value+ " samples ***with pre-defined dB level " +dblevel+ "dB for stripping lead silence***")
		cmd = binpath+ ' -to Test_Results/'+testID+ ' -c 0 -blksz_s '  +value+ ' -thr_db '+dblevel+' -i Test_Signals/' +testsignal
	else:
		value = str(blksz)
		cmd = binpath+ ' -to Test_Results/'+testID+ ' -c 0 -blksz_s '  +value+ ' -i Test_Signals/' +testsignal
		print("Calculating results for '" +testsignal+ "' signal with block size of" +value+ " samples ***with stripping lead silence***")
	#os.system(cmd)
	print(cmd)
	subprocess.call(cmd,shell=True)
	txt2Csv(testID)
	return

## Function to call test cases with different channel mapping
#def callTestMap(testID,binary,testsignal,mapnumber):
#    print("")
#    mapnumber = str(mapnumber)
#    print("Test ID : " +testID)
#    print("Calculating results for '" +testsignal+ "' signal ***with pre-defined channel mapping number: " +mapnumber+ " ***")
#    cmd = 'bin/'+binary+ ' -to Test_Results/'+testID+ ' -c a -m '+mapnumber+' -i Test_Signals/' +testsignal
#    os.system(cmd)
#    txt2Csv(testID)
#    return

  
###################################
# Test Cases
###################################

def amp_vs_time():
	print("Description : This test checks amp_vs_time tool using multi-channel signals")
	print("**All Test cases for this binary are executed with silence stripping (without -s) unless specified in the individual test case description**" )
	print("The expected results may have a 0.01 dB variations and cross platform variation are expected")
	print("")
	print("Test ID 01 to 04")
	callTest('amp_vs_time_01','2_ampswp4k.wav',2)
	callTest('amp_vs_time_02','center.wav',1)
	callTest('amp_vs_time_03','6_41_ampswp.wav',1)
	callTest('amp_vs_time_04','2_ampswp41_48_192.wav',2)
	callxTest('amp_vs_time_05','2_ampswp4k.wav',1,5)
	printCall()
	printCompare()
	print("Please Wait (comparing big files of references and results)...")
	callCompare('amp_vs_time',1,6)
	printResult()
	return

def dyn_rng():
	print("Description - This test checks the dyn_rng tool using 16 bit and 24 bit .wav files and extensible wav header format files")
	print("**All Test cases for this binary are executed with silence stripping (without -s) unless specified in the individual test case description**" )
	print("The expected results may have a 0.01 dB variations and cross platform variation are expected")
	print("")
	print("Test ID 01 to 28")
	callTest('dyn_rng_01','2_200_60_48.wav',2)
	callTest('dyn_rng_02','2_200_60_48_192.wav',2)
	callTest('dyn_rng_03','2_200_60_48_dist1.wav',2)
	callTest('dyn_rng_04','2_200_60_48_dist2.wav',2)
	callTest('dyn_rng_05','dyn_rng_48000.wav',2)
	callTest('dyn_rng_06','dyn_rng_44100.wav',2)
	callTest('dyn_rng_07','dyn_rng_32000.wav',1)
	callTest('dyn_rng_08','2_200_60_48_192.wav',2)
	print("")
	print("***Test ID 11 tests the tool for -thr_db option***")
	callLevelTest('dyn_rng_11','2_200_60_48_192.wav','db',-90,0)
	print("")
	print("***Test ID 12 and 13 test the tool for other sampling rates of 22k and 64k***")
	callTest('dyn_rng_12','test_22K.wav',2)
	callTest('dyn_rng_13','test_64K.wav',2)
	print("")
	print("***Test ID 14 to 18 are for WAV header checking and conformation***")
	callTest('dyn_rng_14','vanilla.wav',1)
	callTest('dyn_rng_15','vanilla-AC3_DLBECHNK.wav',1)
	callTest('dyn_rng_16','vanilla-AC3_DLBECHNK_EX.wav',1)
	callTest('dyn_rng_17','vanillafloat.wav',1)
	callTest('dyn_rng_18','vanilla24bitint.wav',1)
	print("")
	print("***Test ID 19 tests the tool for -s (without stripping silence) option***")
	callnStripTest('dyn_rng_19','2_200_60_48_192.wav')
	print("")
	print("***Test ID 20 to 28 test the tool for different sampling rates for the wav header check***")
	callTest('dyn_rng_20','vanilla_6_32000-AC3_DLBECHNK.wav',3)
	callTest('dyn_rng_21','vanilla_6_32000-AC3_DLBECHNK_EX.wav',3)
	callTest('dyn_rng_22','vanilla_6_32000.wav',3)
	callTest('dyn_rng_23','vanilla_6_44100-AC3_DLBECHNK.wav',3)
	callTest('dyn_rng_24','vanilla_6_44100-AC3_DLBECHNK_EX.wav',3)
	callTest('dyn_rng_25','vanilla_6_44100.wav',3)
	callTest('dyn_rng_26','vanilla_6_48000-AC3_DLBECHNK.wav',3)
	callTest('dyn_rng_27','vanilla_6_48000-AC3_DLBECHNK_EX.wav',3)
	callTest('dyn_rng_28','vanilla_6_48000.wav',3)
	printCall()
	printCompare()
	callCompare('dyn_rng',1,9)
	callCompare('dyn_rng',11,12)
	callCompare('dyn_rng',14,29)
	#callCompare('dyn_rng',20,29)
	printResult()
	return

def freq_resp():
	print("Description - This test checks the freq_resp tool using .wav files with 2 channels, 6 channels and various sampling rates")
	print("**All Test cases for this binary are executed with silence stripping (without -s) unless specified in the individual test case description**" )
	print("The expected results can have a difference of 0.01dB and cross platform variation are expected")
	print("")
	print("Test ID 03 to 26")
	print("***Test ID 03 to 08 test the tool for multi channel signals (6 Channels) and various sampling rates***")
	callTest('freq_resp_03','6_frqstp_32_384.wav',3)
	callTest('freq_resp_04','6_test_frqstp_48000.wav',3)
	callTest('freq_resp_05','6_test_frqstp_32000.wav',3)
	callTest('freq_resp_06','6_frqstp_32_448.wav',3)
	callTest('freq_resp_07','6_frqstp_441_384.wav',3)
	callTest('freq_resp_08','6_frqstp_48_384.wav',3)
	print("")
	print("***Test ID 09 to 14 test the tool for 2 channel signals*** ")
	callTest('freq_resp_09','2_test_frqstp_32000.wav',2)
	callTest('freq_resp_10','2_test_frqstp_48000.wav',2)
	callTest('freq_resp_11','2_test_frqstp_44100.wav',2)
	#callTest('freq_resp_12','2_frqstp_48_24.wav',2) #removed inappropriate wav file test case
	callTest('freq_resp_13','2_frqstp_48_16.wav',2)
	callTest('freq_resp_14','2_frqstp_44_16.wav',2)
	print("")
	print("***Test ID 15 to 16 test the tool for different signals*** ")
	callTest('freq_resp_15','fr_thdvf_dd20-1dB.wav',1)
	callTest('freq_resp_16','fr_thdvf-1dB.wav',1)
	#callTest('freq_resp_17','empty.wav',1) #removed empty file test case
	#print("")
	#print("***Test ID 17 tests the tool for alternate channel mapping***")
	#callTestMap('freq_resp_17','6_frqstp_32_384.wav',6)
	print("")
	print("***Test ID 18 and 19 test the tool for other sampling rates of 22k and 64k***")
	callTest('freq_resp_18','test_22K.wav',2)
	callTest('freq_resp_19','test_64K.wav',2)
	print("")
	print("***Test ID 20 to 22 test the tool for missing dwells***")
	callTest('freq_resp_20','2_deemph_filt_off_48_192.wav',2)
	callTest('freq_resp_21','2_frqstp_441_640.wav',2)
	callTest('freq_resp_22','center.wav',1)
	print("")
	print("***Test ID 23 tests the tool for powermin switch***")
	callLevelTest('freq_resp_23','fr_thdvf_dd_powermin.wav','pm',-79.8,0)
	printCall()
	printCompare()
	callCompare('freq_resp',3,12)
	callCompare('freq_resp',13,17)
	callCompare('freq_resp',20,24)
	printResult()
	return
  
def noise_mod():
	print("Description - This test checks the noise_mod tool using .wav files with different sampling rates and multiple channels")
	print("**All Test cases for this binary are executed with silence stripping (without -s) unless specified in the individual test case description**" )
	print("The expected results may have a 0.01 dB variations and cross platform variation are expected")
	print("")
	print("Test ID 03 to 07")
	callTest('noise_mod_03','6_41_ampswp.wav',3)
	callTest('noise_mod_04','nmod_dd20-192.wav',1)
	#callTest('noise_mod_05','empty.wav',1) # empty file case removed
	print("")
	print("***Test ID 06 and 07 test the tool for other sampling rates of 22k and 64k***")
	callTest('noise_mod_06','test_22K.wav',1)
	callTest('noise_mod_07','test_64K.wav',1)
	printCall()
	printCompare()
	callCompare('noise_mod',3,5)
	printResult()
	return
  
def pwr_vs_time():
	print("Description - This test checks for pwr_vs_time tool using 16 bit,24 bit,24 bit packed int wav files, and extensible wave format files")
	print("**All Test cases for this binary are executed without silence stripping (with -s) unless specified in the individual test case description**" )
	print("Results variation of 0.01 dB tolerated")
	print("")
	print("RF64 tests")
	callPowerTest('pwr_vs_time_70','8_rf64_frqstp_48000.wav',3,'bs',1024)
	callPowerTest('pwr_vs_time_71','8_rf64_frqstp_48000.rf64',3,'bs',1024)
	callLevelTest('pwr_vs_time_72','8_rf64_frqstp_short_48000_32bit.rf64','pmb',-96,1024)
	# callLevelTest('pwr_vs_time_73','8_rf64_frqstp_48000_64bit.rf64','pmb',-96,1024) # 64bit input not supported
	print("Test ID 01 to 67")
	callPowerTest('pwr_vs_time_01','2048noise_1ktone_shortest.wav',1,'bs',1024)
	callPowerTest('pwr_vs_time_02','2048noise_1ktone_shortest.wav',1,'bs',2048)
	callPowerTest('pwr_vs_time_03','2048noise_1ktone_shortest.wav',1,'bt',10)
	callPowerTest('pwr_vs_time_04','2048noise_1ktone_shortest.wav',1,'bt',50)
	callPowerTest('pwr_vs_time_05','2048noise_1ktone_shortest.wav',1,'bt',75)
	callPowerTest('pwr_vs_time_06','2048noise_1ktone_shortest.wav',1,'bt',125)
	callPowerTest('pwr_vs_time_07','2048noise_1ktone_shortest.wav',1,'bt',200)
	callPowerTest('pwr_vs_time_08','2048noise_1ktone_shortest.wav',1,'bt',800)
	callPowerTest('pwr_vs_time_09','6_41_ampswp.wav',3,'bt',500)
	callPowerTest('pwr_vs_time_10','6_41_ampswp.wav',3,'bt',1000)
	callPowerTest('pwr_vs_time_11','6_41_ampswp.wav',3,'bt',2500)
	callPowerTest('pwr_vs_time_12','6_41_ampswp.wav',3,'bt',5000)
	callPowerTest('pwr_vs_time_13','6_41_ampswp.wav',3,'bt',11000)
	callPowerTest('pwr_vs_time_14','6_41_ampswp.wav',3,'bt',111999)
	callPowerTest('pwr_vs_time_15','6_41_ampswp.wav',3,'bs',2048)
	callPowerTest('pwr_vs_time_16','6_41_ampswp.wav',3,'bs',1024)
	#callPowerTest('pwr_vs_time_17','6_41_ampswp.wav',3,'bs',2048) #same as ID15 , Hence removed
	callPowerTest('pwr_vs_time_18','6_41_ampswp.wav',3,'bs',4096)
	callPowerTest('pwr_vs_time_19','center.wav',1,'bt',2000)
	callPowerTest('pwr_vs_time_20','center.wav',1,'bt',7500)
	callPowerTest('pwr_vs_time_21','center.wav',1,'bs',4096)
	callPowerTest('pwr_vs_time_22','6_test_frqstp_32000.wav',3,'bs',1024)
	callPowerTest('pwr_vs_time_23','6_test_frqstp_32000.wav',3,'bs',4096)
	callPowerTest('pwr_vs_time_24','6_test_frqstp_32000.wav',3,'bt',1500)
	callPowerTest('pwr_vs_time_25','6_test_frqstp_32000.wav',3,'bt',5000)
	callPowerTest('pwr_vs_time_26','6_test_frqstp_32000.wav',3,'bt',25000)
	callLevelTest('pwr_vs_time_27','2_ampswp4k.wav','pmb',-110,1024)
	callPowerTest('pwr_vs_time_28','2_ampswp4k.wav',2,'bt',5000)
	callPowerTest('pwr_vs_time_29','2_ampswp4k.wav',2,'bs',4096)
	print("")
	print("***Test ID 36 to 39 test the tool for other sampling rates of 22k and 64k***")
	callPowerTest('pwr_vs_time_36','test_22K.wav',1,'bs',22000)
	callPowerTest('pwr_vs_time_37','test_22K.wav',1,'bt',7500)
	callPowerTest('pwr_vs_time_38','test_64K.wav',1,'bs',32000)
	callPowerTest('pwr_vs_time_39','test_64K.wav',1,'bt',750)
	print("")
	print("***Test ID 40 to 55 test the tool for the wav header performance***")
	callPowerTest('pwr_vs_time_40','vanilla.wav',1,'bt',250)
	callPowerTest('pwr_vs_time_41','vanilla.wav',1,'bs',2400)
	callPowerTest('pwr_vs_time_42','vanilla24bitint.wav',1,'bs',24000)
	callPowerTest('pwr_vs_time_43','vanilla24bitint.wav',1,'bt',250)
	callPowerTest('pwr_vs_time_44','vanilla24bitint.wav',1,'bt',150)
	callPowerTest('pwr_vs_time_45','vanilla-AC3_DLBECHNK.wav',1,'bt',2500)
	callPowerTest('pwr_vs_time_46','vanilla-AC3_DLBECHNK.wav',1,'bs',2400)
	callPowerTest('pwr_vs_time_47','vanilla-AC3_DLBECHNK.wav',1,'bt',500)
	callPowerTest('pwr_vs_time_48','vanilla-AC3_DLBECHNK_EX.wav',1,'bt',1500)
	callPowerTest('pwr_vs_time_49','vanilla-AC3_DLBECHNK_EX.wav',1,'bs',2400)
	callPowerTest('pwr_vs_time_50','vanilla-AC3_DLBECHNK_EX.wav',1,'bt',500)
	callPowerTest('pwr_vs_time_51','vanilla-AC3_DLBECHNK_EX.wav',1,'bs',4800)
	callPowerTest('pwr_vs_time_52','vanillafloat.wav',1,'bs',4800)
	callPowerTest('pwr_vs_time_53','vanillafloat.wav',1,'bt',375)
	#callPowerTest('pwr_vs_time_54','vanilla24bitpacked_20bit.wav',1,'bs',10240) #removed inappropriate wav file test case
	#callPowerTest('pwr_vs_time_55','vanilla24bitpacked_20bit.wav',1,'bt',5000) #removed inappropriate wav file test case
	print("")
	print("***Test ID 54 and 55 test the tool for thr_db and without -s (with stripping silence ) option***")
	callLevelTest('pwr_vs_time_54','center.wav','dbb',-80,4096)
	callLevelTest('pwr_vs_time_55','center.wav','sb',0,4096)

	print("")
	print("***Test ID 56 to 64 check the wav header performance but for diff sampling rates viz 32000,48000 and 44100***")
	callPowerTest('pwr_vs_time_56','vanilla_6_32000-AC3_DLBECHNK.wav',3,'bs',32000)
	callPowerTest('pwr_vs_time_57','vanilla_6_32000-AC3_DLBECHNK_EX.wav',3,'bs',32000)
	callPowerTest('pwr_vs_time_58','vanilla_6_32000.wav',3,'bs',32000)
	callPowerTest('pwr_vs_time_59','vanilla_6_44100-AC3_DLBECHNK.wav',3,'bs',44100)
	callPowerTest('pwr_vs_time_60','vanilla_6_44100-AC3_DLBECHNK_EX.wav',3,'bs',44100)
	callPowerTest('pwr_vs_time_61','vanilla_6_44100.wav',3,'bs',44100)
	callPowerTest('pwr_vs_time_62','vanilla_6_48000-AC3_DLBECHNK.wav',3,'bs',48000)
	callPowerTest('pwr_vs_time_63','vanilla_6_48000-AC3_DLBECHNK_EX.wav',3,'bs',48000)
	callPowerTest('pwr_vs_time_64','vanilla_6_48000.wav',3,'bs',48000)
	print("")
	print("***Test ID 65 to 67 test the tool for the powermin switch for various bit depths viz. 16,24,32 bit.***")
	callLevelTest('pwr_vs_time_65','32_silence_mit_sine.wav','pmb',-100,48000)
	callLevelTest('pwr_vs_time_66','16_silence_mit_sine.wav','pmb',-99,48000)
	callLevelTest('pwr_vs_time_67','24_silence_mit_sine.wav','pmb',-120,48000)
	printCall()
	printCompare()
	callCompare('pwr_vs_time',1,17)
	callCompare('pwr_vs_time',18,30)
	callCompare('pwr_vs_time',36,68)
	callCompare('pwr_vs_time',70,73)
	printResult()
	return
  
def mult_freq_resp():
	print("Description - This test checks the mult_freq_resp tool using a .wav file and a .txt file for multiple channels")
	print("**All Test cases for this binary are executed with silence stripping (without -s) unless specified in the individual test case description**" )
	print("The expected results allow a max deviation of 0.01dB and cross platform variation expected because of different FFT libraries")
	print("")
	print("Test ID 01 to 05")
	callMultTest('mult_freq_resp_01','dv_30_processed_file')
	callMultTest('mult_freq_resp_02','dv_30_processed_file_NSwithDup')
	callMultTest('mult_freq_resp_03','2ch_contour_ne40_24b_48k')
	callMultTest('mult_freq_resp_04','dv_30_processed_file_short')
	callMultTest('mult_freq_resp_05','dv_30_processed_file_shorter')
	printCall()
	printCompare()
	callCompare('mult_freq_resp',1,6)
	printResult()
	return

def spectrum_avg():
	print("Description - This test checks the spectrum_avg tool using .wav files of 22k and 64k sampling rates and multichannel signals")
	print("**All Test cases for this binary are executed with silence stripping (without -s) unless specified in the individual test case description**" )
	print("The expected results can have a difference of 0.01dB and cross platform variations are expected")
	print("")
	print("Test ID 01 to 11")
	callTest('spectrum_avg_02','fftavg_mult_44100.wav',1)
	callTest('spectrum_avg_03','fft_avg_square.wav',1)
	callTest('spectrum_avg_04','fft_avg_lev_check.wav',2)
	#callTest('spectrum_avg_06','empty.wav',1) # empty file case removed
	print("")
	print("***Test ID 07 and 08 test the tool for other sampling rates of 22k and 64k***")
	callTest('spectrum_avg_07','test_22K.wav',1)
	callTest('spectrum_avg_08','test_64K.wav',1)
	print("")
	print("***Test ID 09 and 10 test the tool for multi channel signals***")
	callTest('spectrum_avg_09','6_test_frqstp_32000.wav',3)
	callTest('spectrum_avg_10','6_41_ampswp.wav',3)
	print("")
	print("***Test ID 11 tests the tool for powermin option***")
	callLevelTest('spectrum_avg_11','fftavg_mult_44100_silence.wav','pm',-100,0)
	printCall()
	printCompare()
	callCompare('spectrum_avg',2,5)
	callCompare('spectrum_avg',9,12)
	printResult()
	return

def spectrum_nfft():
	print("Description - This test checks spectrum_NFFT tool using .wav files for various block sizes, channels and averages")
	print("**All Test cases for this binary are executed without silence stripping (with -s) unless specified in the individual test case description**" )
	print("**All Test cases for this binary are executed with window type 'Blackman Harris' unless specified in the individual test case description**" )
	print("")
	print("Test ID 01 to 06")
	callNfftTest('spectrum_NFFT_01_512','2048noise_1ktone_shortest.wav',1,512,'n')
	callNfftTest('spectrum_NFFT_01_2048','2048noise_1ktone_shortest.wav',1,2048,'n')
	callNfftTest('spectrum_NFFT_01_8192','2048noise_1ktone_shortest.wav',1,8192,'n')
	callNfftTest('spectrum_NFFT_02_512','fftavg_mult_44100.wav',1,512,'n')
	callNfftTest('spectrum_NFFT_02_1024','fftavg_mult_44100.wav',1,1024,'n')
	callNfftTest('spectrum_NFFT_02_4096','fftavg_mult_44100.wav',1,4096,'n')
	callNfftTest('spectrum_NFFT_02_8192','fftavg_mult_44100.wav',1,8192,'n')
	callNfftTest('spectrum_NFFT_02_10240','fftavg_mult_44100.wav',1,10240,'n')
	callNfftTest('spectrum_NFFT_03_512','6_test_frqstp_32000.wav',3,512,'n')
	callNfftTest('spectrum_NFFT_03_1024','6_test_frqstp_32000.wav',3,1024,'n')
	callNfftTest('spectrum_NFFT_03_4096','6_test_frqstp_32000.wav',3,4096,'n')
	callNfftTest('spectrum_NFFT_03_8192','6_test_frqstp_32000.wav',3,8192,'n')
	callNfftTest('spectrum_NFFT_03_10240','6_test_frqstp_32000.wav',3,10240,'n')
	callNfftTest('spectrum_NFFT_04_512','321_48_16_sine_tones_cplbndstrc_at_448_kbps.wav',3,512,'n')
	callNfftTest('spectrum_NFFT_04_1024','321_48_16_sine_tones_cplbndstrc_at_448_kbps.wav',3,1024,'n')
	callNfftTest('spectrum_NFFT_04_4096','321_48_16_sine_tones_cplbndstrc_at_448_kbps.wav',3,4096,'n')
	callNfftTest('spectrum_NFFT_04_8192','321_48_16_sine_tones_cplbndstrc_at_448_kbps.wav',3,8192,'n')
	callNfftTest('spectrum_NFFT_04_10240','321_48_16_sine_tones_cplbndstrc_at_448_kbps.wav',3,10240,'n')
	print("")
	print("***Test ID 05 and 06 test the tool for powermin option***")
	callNfftTest('spectrum_NFFT_05_512','321_48_16_sine_tones_cplbndstrc_at_448_kbps.wav',3,512,-100)
	callNfftTest('spectrum_NFFT_05_1024','321_48_16_sine_tones_cplbndstrc_at_448_kbps.wav',3,1024,-100)
	callNfftTest('spectrum_NFFT_05_4096','321_48_16_sine_tones_cplbndstrc_at_448_kbps.wav',3,4096,-100)
	callNfftTest('spectrum_NFFT_05_8192','321_48_16_sine_tones_cplbndstrc_at_448_kbps.wav',3,8192,-100)
	callNfftTest('spectrum_NFFT_05_10240','321_48_16_sine_tones_cplbndstrc_at_448_kbps.wav',3,10240,-100)
	callNfftTest('spectrum_NFFT_06_512','fftavg_mult_44100.wav',1,512,-99)
	callNfftTest('spectrum_NFFT_06_1024','fftavg_mult_44100.wav',1,1024,-99)
	callNfftTest('spectrum_NFFT_06_4096','fftavg_mult_44100.wav',1,4096,-99)
	callNfftTest('spectrum_NFFT_06_8192','fftavg_mult_44100.wav',1,8192,-99)
	callNfftTest('spectrum_NFFT_06_10240','fftavg_mult_44100.wav',1,10240,-99)
	print("")
	print("***Test ID 07 to 12 test the tool for -z (navg) and -window option***")
	callNfftSplTest('spectrum_NFFT_07','6_test_frqstp_32000.wav',512,5000,'z')
	callNfftSplTest('spectrum_NFFT_08','fftavg_mult_44100.wav',512,1,'w')
	callNfftSplTest('spectrum_NFFT_09','fftavg_mult_44100.wav',512,2,'w')
	callNfftSplTest('spectrum_NFFT_10','fftavg_mult_44100.wav',512,4,'w')
	callNfftSplTest('spectrum_NFFT_11','fftavg_mult_44100.wav',512,5,'w')
	callNfftSplTest('spectrum_NFFT_12','fftavg_mult_44100.wav',512,6,'w')
	#callNfftSplTest('spectrum_NFFT_02_512','fftavg_mult_44100.wav',512,10,'w')
	print("")
	print("***RF64 tests***")
	callNfftTest('spectrum_NFFT_13','8_rf64_frqstp_short_48000.wav',3,1024,-100)
	printCall()
	printCompare()
	resultCompare('spectrum_NFFT_01_512')
	resultCompare('spectrum_NFFT_01_2048')
	resultCompare('spectrum_NFFT_01_8192')
	callCompareNfft(2,7)
	callCompare('spectrum_NFFT',7,13)
	resultCompare('spectrum_NFFT_13')
	printResult()
	return

def thd_vs_freq():
	print("Description - This test checks the thd_vs_freq tool using various samping rates, multichannel signals and missing frequency dwells")
	print("**All Test cases for this binary are executed with silence stripping (without -s) unless specified in the individual test case description**" )
	print("The expected results have a variation of 0.01dB and cross platform variation is expected")
	print("")
	print("Test ID 03 to 23")
	print("")
	print("***Test ID 03 to 08 test the tool for multi channel signals and various sampling rates***")
	callTest('thd_vs_freq_03','6_frqstp_32_384.wav',3)
	callTest('thd_vs_freq_04','6_test_frqstp_48000.wav',3)
	callTest('thd_vs_freq_05','6_test_frqstp_32000.wav',3)
	callTest('thd_vs_freq_06','6_frqstp_32_448.wav',3)
	callTest('thd_vs_freq_07','6_frqstp_441_384.wav',3)
	callTest('thd_vs_freq_08','6_frqstp_48_384.wav',3)
	print("")
	print("***Test ID 09 to 14 test the tool 2 channel signals***")
	callTest('thd_vs_freq_09','2_test_frqstp_32000.wav',2)
	callTest('thd_vs_freq_10','2_test_frqstp_48000.wav',2)
	callTest('thd_vs_freq_11','2_test_frqstp_44100.wav',2)
	#callTest('thd_vs_freq_12','2_frqstp_48_24.wav',2) #removed inappropriate wav file test case
	callTest('thd_vs_freq_13','2_frqstp_48_16.wav',2)
	callTest('thd_vs_freq_14','2_frqstp_44_16.wav',2)
	print("")
	print("***Test ID 15 and 16 test the tool for diff signals***")
	callTest('thd_vs_freq_15','fr_thdvf_dd20-1dB.wav',1)
	callTest('thd_vs_freq_16','fr_thdvf-1dB.wav',1)
	#callTest('thd_vs_freq_17','empty.wav',1) #removed empty file test case
	print("")
	print("***Test ID 18 and 19 test the tool for other sampling rates of 22k and 64k***")
	callTest('thd_vs_freq_18','test_22K.wav',1)
	callTest('thd_vs_freq_19','test_64K.wav',1)
	print("")
	print("***Test ID 20 to 22 test the tool for missing dwells***")
	callTest('thd_vs_freq_20','2_deemph_filt_off_48_192.wav',2)
	callTest('thd_vs_freq_21','2_frqstp_441_640.wav',2)
	callTest('thd_vs_freq_22','center.wav',1)
	print("")
	print("***Test ID 23 tests the tool for powermin switch***")
	callLevelTest('thd_vs_freq_23','fr_thdvf_dd20-1dB_silence.wav','pm',-99,0)
	printCall()
	printCompare()
	callCompare('thd_vs_freq',3,12)
	callCompare('thd_vs_freq',13,17)
	callCompare('thd_vs_freq',20,24)
	printResult()
	return

def thd_vs_level():
	print("Description - This test checks the thd_vs_level tool using .wav files that are multichannel, have different sampling rates and amplitude sweep")
	print("**All Test cases for this binary are executed with silence stripping (without -s) unless specified in the individual test case description**" )
	print("The expected results have a 0.01dB deviation and cross platform variation is expected")
	print("")
	print("Test ID 01 to 12")
	callTest('thd_vs_level_01','2_4K_ampswp.wav',2)
	callTest('thd_vs_level_02','2_4K_ampswp_32000.wav',2)
	callTest('thd_vs_level_03','2_4K_ampswp_44100.wav',2)
	callTest('thd_vs_level_04','2_ampswp41_48_192.wav',2)
	callTest('thd_vs_level_05','2_ampswp4k.wav',2)
	callTest('thd_vs_level_06','2_ampswp4k_48_192.wav',2)
	callTest('thd_vs_level_07','thdvl_dd20-192.wav',1)
	#callTest('thd_vs_level_08','empty.wav',1) #removed empty file test case
	print("")
	print("***Test ID 09 and 10 test the tool for other sampling rates of 22k and 64k***")
	callTest('thd_vs_level_09','test_22K.wav',1)
	callTest('thd_vs_level_10','test_64K.wav',1)
	printCall()
	printCompare()
	callCompare('thd_vs_level',1,8)
	printResult()
	return
###################################
# Main Function
###################################  

# Recursive options 
def options():
	chooseVer()
	chooseBin()
	return


def main():
	#os.system('cls')
	#os.system('clear')
	checkPyVer()
	clearFiles('Test_Results/Error_Difference/*.csv')
	choosePlt()
	options()  
	return

if __name__=="__main__":
	main()
