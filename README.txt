Flow Balanced Bipartitioner

Written by Alan Bannon, Spring 2018, for ECE6133 Physical Design Automation of VLSI systems
at Georgia Tech.

Distributed under the Beerware license (Rev 42), full text at end of file.



Project was built in Clion IDE.
Building and running instructions assume use of a bash shell. If you're using
Windows command line, use GCC 5 or later. No compiler flags should be required. 

--Building:-----------------------------------------------------------------------------

Navigate to program directory and run commands:

$ cmake .

$ make

$ ./FBB_base

// Note: you may get a clock-skew warning on running make. This seems to be an issue
// with using an SSH session over Gatech's VPN and does not affect the build process.


---Running:-------------------------------------------------------------------------------

Navigate to program directory and run commands:

$./FBB_base

The FBB launcher will then request a filepath to the target file.
Note that the file read function is NOT ROBUST and will happily parse any file you give it.
Giving a path to a non-.blif file may cause a crash, so only feed it .blif files.
`
The launcher will request target solution ratio and solution ratio deviation.

The launcher will give the option of running in verbose mode to show the progress of the
algorithm.

The launcher will request a number of runs of the algorithm to perform. This is useful in 
benchmarking when multiple runs must be performed. For any other purpose use a value of 1.

---filepaths--------------------------------------------------------------------------------

For convenience a list of filepaths to the benchmark files is included:
benchmark_files/s9234.blif		
benchmark_files/s13207.blif		
benchmark_files/b22_opt.blif	
benchmark_files/b20_opt.blif	
benchmark_files/b17_opt.blif


benchmark_files/testModel.blif
// This file was generated from the example circuit
// on page 50 of Practical Problems in VLSI Phyiscal
// Design Automation by Professor Sung Kyu Lim.


/*
 * ------------------------------------------------------------
 * "THE BEERWARE LICENSE" (Revision 42):
 * Alan Bannon wrote this code. As long as you retain this
 * notice, you can do whatever you want with this stuff. If we
 * meet someday, and you think this stuff is worth it, you can
 * buy me a beer in return.
 * ------------------------------------------------------------
 */