# Assembler-and-CPU-Simulator

This is a FISC Assembler that takes in the assembly file, parses each line from the file, and converts it into machine code then to a hexadecimal number. The .hex file is the output file that contains all the hexadecimal numbers. The .s file must already exist for the assembler to work.

The SISC assembler basically takes in six or seven instructions instead of FISC which only takes in four or five instructions. 

There are simulators written in C++ to read in the HEX output file that was created from the assembler. It will simulate each instruction by modifying the variables that represent the state elements (registers) of the CPU.

More information about the program is explained in the FISC & SISC pdf. The outputfiles folder will have the file moreinfo.txt which is helpful for knowing how to compile and run program.
