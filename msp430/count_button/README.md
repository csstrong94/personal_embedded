can be compiled with GNU make

targets are 

* build
* clean
* fram-write
* %.i
* %.o

fram-write will use MSPFlasher 1.3.20 to erase all memory on MSP430 and write to FRAM the created .hex executable
generated during the build process. Example workflow is below:


make build       # compile and link 
make fram-write  # write to MSP430 FRAM


You can modify the MSPFlasher exit flags to change the behavior of the board after flashing is complete. By default VCC is applied to the board (3 V) 
and the board will power up and begin running the new program


