# RISC-V Simulator
This is a cycle accurate RISC-V RV32I simulator built in C++. Its primary objective is to load up binary inputs and spit an output with changes in register files and to log performance metrics. This project is for learning purposes, (mine and anyone else who wishes to use this) and only has a single stage pipeline built into it, I will be including a 5 stage pipeline in the future.

# How to run
In the terminal, you only have to provide the file path for the specified C++ file

```
PS D:\code> g++ CPPcode.cpp -o run.exe
PS D:\code> .\run.exe
Enter path containing the memory files: D:\code
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
