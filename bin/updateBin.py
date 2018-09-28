import sys
import glob
import os
import shutil

binplt = ''
stdiover = ''
lib = 'kiss'

def checkPyVer():
	if sys.version_info.major > 2:
		py_ver = str(sys.version_info.major)+'.'+str(sys.version_info.minor)
		print('Python version '+py_ver+' found')
		print('Please use version 2.7')
		print('Closing down ..')
		sys.exit()
	return

def clearFiles(filepath):
	files = glob.glob(filepath)
	for f in files:
		os.remove(f)
	return


#Choice of Platform
def choosePlt():
	global binplt
	print("################################\n")
	print("Choose the platform in which the binaries are built : \n")
	print("NOTE! The bin folder contains only copies of binaries/executables which must be already built in the individual tool folder under the make folder.\nIncase the tools are updated and built again, this script needs to be run to update the bin folder with the latest binaries/Executables." )
	print("WARNING ! The existing binaries or executables currently in the bin folder will be updated with your choice.")
	print("################################\n")
	print("")
	print("[1]linux_amd64_gnu\n[2]linux_x86_gnu\n[3]windows_x86_gnu\n[4]windows_amd64_msvs\n[5]windows_x86_msvs\n[6]exit\n")

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
		sys.exit()
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
		sys.exit()
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
			sys.exit()
	return

def copyBin(binname,l):
	global binplt
	global binver
	binplt_tmp = binplt
	if (binplt_tmp == 'windows_amd64_msvs' or binplt_tmp == 'windows_x86_msvs'):
		binplt_tmp = os.path.join(binplt_tmp,'release',stdiover)
	if l == 'intel':
		binpath=os.path.join('..','make',binname+'_intel',binplt_tmp)
	else:
		binpath = os.path.join('..','make',binname,binplt_tmp)
	source = os.listdir(binpath)
	for files in source:
		if (binplt == 'windows_amd64_msvs' or binplt == 'windows_x86_msvs'):
			if files.endswith(".exe"):
				shutil.copy(os.path.join(binpath,files),os.getcwd())
		else:
			if files.endswith("release.exe") or files.endswith("release") :
				shutil.copy(os.path.join(binpath,files),os.getcwd())
	return

def main():
	checkPyVer()
	clearFiles('*.exe')
	clearFiles('*release')
	choosePlt()
	chooseLib()
	copyBin('amp_vs_time','')
	copyBin('dyn_rng','')
	copyBin('freq_resp','')
	copyBin('mult_freq_resp',lib)
	copyBin('noise_mod','')
	copyBin('pwr_vs_time','')
	copyBin('spectrum_avg',lib)
	copyBin('spectrum_NFFT',lib)
	copyBin('thd_vs_freq','')
	copyBin('thd_vs_level','')
	return

if __name__=="__main__":
	main()
