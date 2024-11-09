# RISC-V Simulator
This is a cycle accurate RISC-V RV32I simulator built in C++. Its primary objective is to load up binary inputs and spit an output with changes in register files and to log performance metrics. This project is for learning purposes, (mine and anyone else who wishes to use this) and only has a single stage pipeline built into it, I will be including a 5 stage pipeline in the future.

# How to run
In the terminal, you only have to provide the file path for the specified C++ file

```
.\run.exe
Enter path containing the memory files: <filepath>
```
This will generate all output files in the same location. Make sure your imem.txt and dmem.txt files are in the same location as the cpp file

Its a windows output as I built it on windows, and everything other than the file directory system should be the same for Linux or Mac

# Input(example)
Here you have to modify the imem.txt file present in the inputs folder, with your own RISC-V instructions encoded in binary. It reads from left to right and takes 8 bits per line

![image](https://github.com/user-attachments/assets/d842c92c-e776-4102-99b5-2f8ef1a8e2e5)


For the instruction
```
LW R1, R0, #0
LW R2, R0, #4
SUB R3, R1, R2
SW R3, R0, #8
HALT
```
Modify the imem file accordingly with respect to the instructions you want to load, into the simulator.<br>
The final 32 bits represent a HALT, which toggles a flip-flop into breaking up the instruction cycle.

# Ouput(example)

This only shows the RFRegister file, which is the most visible and important output file

![image](https://github.com/user-attachments/assets/626f028f-ab21-4a01-a02e-64c2def5b337)

Shows the final state of the register file with all modified registers when the code has completed execution

# Performance
Tracks the performance of the processor

![image](https://github.com/user-attachments/assets/fc56f136-1e9c-493f-ac19-ce08e729c0c7)




