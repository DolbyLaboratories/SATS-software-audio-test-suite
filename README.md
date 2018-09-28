# Dolby® SATS (Software Audio Test Suite)

## Introduction
The Software Audio Test Suite (SATS) is a collection of software audio test tools for testing digital audio products or devices. It consists of:

- amp_vs_time (Amplitude versus time)
- dyn_rng (Dynamic Range)
- freq_resp (Frequency Response)
- multi_freq_resp (Multiple Frequency Response)
- noise_mod (Noise Modulation)
- pwr_vs_time (Power versus Time)
- spectrum_avg (Spectrum Averaging)
- spectrum_NFFT (Spectrum of NFFT points)
- thd_vs_freq (Total Harmonic Distortion plus Noise versus Frequency)
- thd_vs_level (Total Harmonic Distortion plus Noise versus Level)

These audio test tools are designed to be run from a PC command line (Command Prompt) or shell (such as Bash), and operate on .wav files. Alternatively, the audio test tools can be incorporated into scripts for automation purposes. A test framework and a collection of test signals and reference results are provided for checking limits, so that simple pass/fail decisions can be generated. The common test framework is designed to test every tool provided with the package generated using either a Linux or Windows platform. The SATS tools mult_freq_resp, spectrum_avg and spectrum_NFFT require FFT calculations. These FFT calculations are provided by the KISS_FFT library package which is included in SATS and can also be downloaded at https://sourceforge.net/projects/kissfft/. As a powerful alternative to KISS_FFT, SATS also supports the INTEL® Math Kernel Library (MKL) FFT (for Linux and Windows MSVS) for INTEL® platforms. But this library has to be downloaded separately and organized as per the details mentioned in the Prerequisites -> Libraries section.
The following tables provide an overview of supported platforms for the KISS_FFT and Intel® MKL libraries:

----
| Platform     | linux_amd_64 | linux_x86_gnu | windows_x86_gnu | windows_amd64_msvs (2015) |
| --------     | :----------: | :-----------: | :-------------: | :-----------------------: |
| `KISS_FFT`   |	Yes	        |	Yes         |    Yes          |          Yes            | 
| `Intel® MKL` |	Yes	        | 	Yes         |    No           |          Yes            |  
----
| Platform   | windows_x86_msvs (2015) | windows_amd64_msvs (2017) | windows_x86_msvs (2017) |
| --------   | :---------------------: | :-----------------------: | :---------------------: | 
|`KISS_FFT`  |           Yes           |            Yes            |           Yes           |
|`Intel® MKL`|           Yes           |            Yes            |           Yes           |
----

A detailed description of the individual audio test tools and their usage can be found in the pdf document ['Software Audio Test Suite User’s Guide'](SATS_User_Guide_1_0.pdf).


## Prerequisites


### Prerequisites - Building with Linux ( amd64 or x86)

The GNU GCC Compiler must be installed.


### Prerequisites - Building with Windows GNU

Install the complete MinGW package which comes with mingw32 and msys from [here](https://sourceforge.net/projects/mingw/files/). If you install the MinGW Installation Manager, ensure that you install the MinGW Base Package. You can install mingw32 alone, but we recommend installing the full package.
Edit the path variable ( My Computer/ This PC (right click) -> Properties -> Advanced System Settings -> Environment Variables -> System Variables ( Find variable 'Path') -> Edit -> New) and add the following:

```
C:\MinGW\msys\1.0\bin

C:\MinGW\bin
```


### Prerequisites - Building with Visual Studio (2015 or 2017) through command line

This section can be skipped if you are building the tools through Visual Studio IDE.

##### Required
In the *make* folder, you can find two batch scripts *env_var_VS2015.bat* and *env_var_VS2017.bat* that automatically create the Visual Studio Build Environment in a windows command-line shell. **You do not need to set the path/environment variable (described as optional below) if you are making use of these batch scripts.**
The batch scripts run the *VsDevCmd.bat* file found in the MSVS root folder. The standard path to the *VsDevCmd.bat* for 2015 and 2017 versions are different and hence there are two batch files. **For VS2017, ensure that the 'Visual Studio Build Tools 2017' is installed through the VS installer.** *The Visual Studio Build Tools allows you to build native and managed MSBuild-based applications without requiring the Visual Studio IDE.*

The master_project and solution files through command line (msbuild) will make use of the default VS SDK ,i.e, 8.1 SDK. Make sure you have installed the following through the VS installer :

Visual Studio Installer -> Visual Studio 20xx -> Modify -> Workloads -> Desktop development with C++ -> Summary -> *Windows 8.1 SDK and UCRT SDK*

Visual Studio Installer -> Visual Studio 20xx -> Modify -> Individual Components -> SDKs, libraries, and frameworks -> *Windows 8.1 SDK*  

##### Optional 

Edit the path variable ( My Computer/ This PC (right click) -> Properties -> Advanced System Settings -> Environment Variables -> System Variables (Find variable 'Path') -> Edit -> New) and add the path to MSBuild bin folder.

Path would be something like this : C:\Program Files (x86)\Microsoft Visual Studio\2017\Professional\MSBuild\15.0\Bin    (for Professional VS2017) 

### Prerequisites - Using Intel Libraries for FFT calculations

A complete KISS_FFT package is available with the SATS. 
However, if you prefer using Intel® MKL (applicable only when building on Intel-Based Systems), then you should download the library package from Intel® [here](https://software.intel.com/en-us/mkl). 

After the installation process of MKL, copy the files listed below from the Installation path (for example, for Microsoft Windows, you might need to look into this path C:\Program Files (x86)\IntelSWTools\compilers_and_libraries_2018.x.xx\windows\ -> mkl (for mkl_intel_xx.lib) / compiler (for libiomp5md.lib)) and create a folder structure with intel files parallel to the *KISS_FFT* structure :

```
project

| ...

|_ intel_mkl

|  | include       (copy contents: mkl_dft.h; mkl_service.h; mkl_types.h)

|  | linux_amd64   (copy contents if using linux: libmkl_core.a; libmkl_gnu_thread.a; 

					libmkl_intel_lp64.a; libmkl_intel_thread.a; libmkl_sequential.a)

|  | linux_x86     (copy contents if using linux: libmkl_core.a; libmkl_gnu_thread.a; 

					libmkl_intel.a; libmkl_intel_thread.a; libmkl_sequential.a)

|  | windows_amd64 (copy contents if using windows MSVS: libiomp5md.lib; mkl_core.lib; 

					mkl_intel_lp64.lib; mkl_intel_thread.lib)

|  | windows_x86   (copy contents if using windows MSVS:libiomp5md.lib; mkl_core.lib; 

					mkl_intel_c.lib; mkl_intel_thread.lib; mkl_sequential.lib)

|_kiss_fft130

| ...
```

Note: For Linux, you should create a folder structure with *include*, and both *linux_amd64* and *linux_x86*. The master makefile, when used to generate tools with intel library, attempts to generate tools for both these platforms simultaneously and will throw an error if either folder is missing. The same applies for windows MSVS (*include* + *windows_amd64* + *windows_x86*).

Additionally, for windows, you need to edit the Path in the environment (System) variables and add the path to the INTEL® MKL compiler and mkl *.dll* files. For example, for Microsoft Windows amd64, you might need to add these paths: C:\Program Files (x86)\IntelSWTools\compilers_and_libraries_2018.x.xx\windows\redist\intel64_win\compiler\ *and* C:\Program Files (x86)\IntelSWTools\compilers_and_libraries_2018.x.xx\windows\redist\intel64_win\mkl\. For x86, you will need to replace 'intel64_win' in the path address with 'ia32_win'.

### Prerequisites - Running the test framework

Python 2.7 is required for running the test script of the provided test framework and other python scripts.

Note: If your system has both python 2.7 and 3.xx installed, please ensure that you are using the 2.7 version for running the script. You can also force the interpreter to use this version by calling out the complete root path of Python (eg. for Windows) :

```
C:\Python27\python testScript.py
C:\Python27\python updateBin.py
```

## Getting Started

The SATS package comes with individual makefiles for every tool and for each platform. However, if you are interested in generating all tools for a particular platform and for a particular library,  you can make use of the master makefile ( for linux GNU or windows GNU using MinGW ) or project file (for MSVS 2015 or 2017) available in the make folder.


### Getting Started - Building the tools

All makefiles for generating individual binaries or executable are available in the respective folders under make. Eg for amp_vs_time :

```

project

|_ bin

|  |_ updateBin.py			 (run this script to gather copies of binaries/executables under 

							  the bin folder)

|  | ...					  (copies of the binaries/executables) 

| ...

|_ make

|  |_ amp_vs_time   

|  |  | linux_amd64_gnu       (contains makefile and debug+release binaries are generated here)

|  |  | linux_x86_gnu         (contains makefile and debug+release binaries are generated here)

|  |  |_ windows_amd64_msvs   (contains solution files for 2015, 2017)

|  |  |  |_ debug	          (only generated when executables are built)

|  |  |  |  | VS2017          (debug executable for VS2017)

|  |  |  |  | VS2015          (debug executable for VS2015)

|  |  |  | release

|  |  | windows_x86_gnu       (contains makefile and debug+release executables are generated here)

|  |  |_ windows_x86_msvs     (contains solution files for 2015, 2017)

|  |  |  | debug

|  |  |  | release

|  | ...

|  |_ MakeFile                    (Master makefile to generate all windows gnu or linux tools using 

									Kiss or Intel_MKL library)

|  |_ master_libIntel_2015.proj	  (Master projectfile to generate all tools using intel MKL library 

									where necessary for VS2015)	

|  |_ master_libIntel_2017.proj

|  |_ master_libKiss_2015.proj

|  |_ master_libKiss_2017.proj

|  | ...

| ...
```

For binaries/executables that involve FFT calculations, there will be two folders under make (for example: mult_freq_resp , mult_freq_resp_intel)

**All built binaries or executables will be stored in folders with their respective tool names under the *make* folder as shown in the structure above.**  
Running the script *updateBin.py* will gather the copies of the release version of the built binaries/executables from their respective folders and put them all together in the *bin* folder. 
The user will be prompted to choose the platform when the *updateBin.py* script is run. Only tools for a single platform will be stored in the *bin* folder.
Re-running the script will remove the current copies and update the copies with the tools for the new platform.
Note that the testScript and all other files reference or use tools at the original locations under the *make* folder. 
Everytime the binaries/executables are updated or rebuilt, run the *updateBin.py* to get the latest version of the tools in your *bin* folder.

----
#### Getting Started - Linux

Ensure that you have changed to the *make* folder.

For generating individual binaries (debug + release) : 

```
make 
(make clean) 
```

For generating all binaries for linux (amd64 + x86 ; debug + release) through master makefile : 

###### Using KISS_FFT library :

```
make linux_kiss_all 
(make linux_clean)
```

###### Using Intel_MKL library:

```
make linux_intel_all  
(make linux_clean)
```

----
#### Getting Started - Windows GNU ( only x86 and KISS_FFT library supported)

Ensure that you have changed to the *make* folder.

For generating individual executables (debug + release): 

```
set CC=gcc
make  
(make clean)  
```

*or*

```
set CC=gcc
mingw32-make 
(mingw32-make clean)
```

For generating all executables (debug + release) for windows gnu through master makefile : 

```
make windows_kiss_all
(make windows_clean) 
```

*or*

```
mingw32-make windows_kiss_all 
(mingw32-make windows_clean)
```

----
#### Getting Started - Windows MSVS

Ensure that you have changed to the *make* folder.

##### Without using IDE ( through command prompt)

For generating individual executables (eg. for 2017 - amp_vs_time - windows_amd64 - release) : 

Run the batch file *env_var_VS2017.bat*. This opens up a command line window with Visual Studio Build Environment.
```
cd amp_vs_time\windows_amd64_msvs
msbuild amp_vs_time_2017.sln /p:Configuration=Release
(msbuild amp_vs_time_2017.sln /p:Configuration=Release /t:clean)  
(msbuild amp_vs_time_2017.sln /p:Configuration=Release /t:rebuild)
```
Note: For Debug version, use /p:Configuration=Debug

For generating all executables for windows msvs (eg. for 2017 - x86+amd64 - debug+release - using intel) through master project file: 

Run the batch file *env_var_VS2017.bat*. This opens up a command line window with Visual Studio Build Environment.
```
msbuild master_libIntel_2017.proj 
(msbuild master_libIntel_2017.proj /t:clean)
(msbuild master_libIntel_2017.proj /t:rebuild)
```

Note: The equivalent master project file for building tools using KISS FFT would be *master_libKiss_2017.proj*

##### Using IDE

Only Individual executables can be generated at one time. You have an option to use 'retarget solution' if you wish to update to the latest SDK before generating the executable. 

----
#### Getting Started - macOS

The make files for linux can probably be used without changes for macOS. However, this has not been extensively tested.


## Testing the tools using the Test Framework

The Test framework comprises of the following structure :

```
project

| ...

|_ TestFramework

|  |_ Test_Results

|  |  | csvFiles            

|  |  | Error_Difference

|  |  |_ references

|  |  |  | txt2csv.py

|  |  |  | ... 

|  |  | referencesCSV

|  |  | amp_vs_time_01

|  |  | ...

|  | Test_Signals

|  | testScript.py

| ...
```

The testScript.py consists of a series of tests designed for testing different tools ; platforms; libraries. The user will be asked to input choices for these options.

All command line options available for an indiviual tool are tested. Every test ID displays a short description of the test case on the command line. The test results are stored in the *Test_Result* folder and are simultaneously converted to csv files and stored in the *csvFiles* folder. 

These CSV files are then compared against the pre-computed *referencesCSV* files. Any  errors are displayed on the  command line and the differences between the actual result and pre-computed result are stored in the *Error_Difference* folder. 

Note that only failed test cases wil be stored in *Error_Difference*. If the result is an empty file or if the result is computed for less channels than the reference file, the error difference file will not be computed. However, an error message "Some or all channels missing from the result" will be displayed.
 

The tests can be performed for individual binaries or executables after choosing the platform and version (debug or release). There is also an option for the user to test all the binaries/executables for a given platform and version.

If the test all option is selected, the binaries/executables will be tested successively in the order displayed during selection.

When there is a failure in any test case for a binary/executable, the complete test report of that binary/executable will be displayed along with the failed test IDs,  testing will halt. The successive binaries/executables will not be tested unless the issue is fixed.

Note that you can always choose an individual binary/executable to be tested if you want to skip some failed test cases in a particular binary/executable.

If new test cases are added, the user can add the result in the *references* folder and run the python script *txt2csv.py*, which will convert this reference to a CSV file and store it in *referencesCSV*. Note: The test script contains all test cases needed to completely test the binary. Addition of any new test caseS is to be done only if necessary. 

Calling testScript : 

```
python testScript.py
```

## Version
*Release* : GitHub
*Version* : 1.0

## Authors
*Dolby Laboratories Inc.*

## License
Copyright (c) 2018, Dolby Laboratories Inc. - see the LICENSE.txt file for details









