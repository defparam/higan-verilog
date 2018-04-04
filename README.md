higan-verilog - Emulator/Verilog Co-simulation
===============================

Welcome! This repo is a clone of higan v106 modified to provide an example and framework of an Emulator/Simulator co-simulation model. As far as I know this is the first attempt to combine the Verilator engine into a software emulator.

In this example we extend the Super Famicom emulation model at memory-inline.hpp. The Bus::read and Bus::write routines have been modified to insert a proxy verilog model inline. All SNES reads and writes are pushed through this proxy verilog model into a class called verilog-device. The verilog-device class contains all the verilator model hooks to take all read/write operations and push it through a memory snoop model described in verilog (memsnoop.v).

We show an example of memsnoop.v being used to modify specific memory accesses an show how the verilog model effects the emulator.

Here is a video of the memsnoop.v model and Super Mario World: http://youtube.com/T88LhuoQ7pg

A couple things to note:
1) This architecture is not limited to higan, SNES or Verilator. Co-simulation has existed for a while in many RTL simulators. This modeling can be done on other emulators/platforms/roms. I only use higan and memsnoop on SMW to show as an example
2) One can use this architecture to build a co-processor in verilog and test it on emulator
3) One can use this architecture to validate emulation or RTL verilog co-processor models with each other

Limitations
===============================
1) I only tested this on Ubuntu (however a Windows port may be possible)
2) The GNUMakefile changes I made are bad. I am terrible at Makefile. Also for some reason verilator likes to generate all its *.h *.cpp *.o in the same obj directory. If someone would like to extend this and create a better Makefile then I welcome it!

How to install
===============================

1) Install Verilator by following steps here: https://www.veripool.org/projects/verilator/wiki/Installing
2) Install higan pre-reqs: 
	sudo apt-get install build-essential libgtk2.0-dev libpulse-dev \
    mesa-common-dev libgtksourceview2.0-dev libcairo2-dev libsdl1.2-dev \
    libxv-dev libao-dev libopenal-dev libudev-dev
3) Install gtkwave: sudo apt-get install gtkwave
4) make -C higan
5) make -C icarus

The current model of memsnoop.v passes all bus accesses without modification. Feel free to modify the verilog and re-compile higan.




	
	