#include<iostream>
#include<string>
#include<vector>
#include<bitset>
#include<fstream>
#include<cstdint>
#include<iomanip>

using namespace std;

#define MemSize 1000 

struct IFStruct {
    bitset<32>  PC;
    bool        nop;  
};

struct IDStruct {
    bitset<32>  Instr;
    bool        nop;  
};

struct EXStruct {
    bitset<32>  Read_data1;
    bitset<32>  Read_data2;
    bitset<32>  ALUresult;
    bitset<16>  Imm;
    bitset<5>   Rs;
    bitset<5>   Rt;
    bitset<5>   Wrt_reg_addr;
    bool        is_I_type;
    bool        rd_mem;
    bool        wrt_mem; 
    bool        alu_op;     //1 for addu, lw, sw, 0 for subu 
    bool        wrt_enable;
    bool        nop;  
};

struct MEMStruct {
    bitset<32>  ALUresult;
    bitset<32>  Store_data;
    bitset<5>   Rs;
    bitset<5>   Rt;    
    bitset<5>   Wrt_reg_addr;
    bool        rd_mem;
    bool        wrt_mem; 
    bool        wrt_enable;    
    bool        nop;    
};

struct WBStruct {
    bitset<32>  Wrt_data;
    bitset<5>   Rs;
    bitset<5>   Rt;     
    bitset<5>   Wrt_reg_addr;
    bool        wrt_enable;
    bool        nop;     
};

struct stateStruct {
    IFStruct    IF;
    IDStruct    ID;
    EXStruct    EX;
    MEMStruct   MEM;
    WBStruct    WB;
};

class InsMem
{
	public:
		string id, ioDir;
        InsMem(string name, string ioDir) {       
			id = name;
			IMem.resize(MemSize);
            ifstream imem;
			string line;
			int i=0;
			imem.open(ioDir + "\\imem.txt");
			if (imem.is_open())
			{
				while (getline(imem,line))
				{
                    cout << "Reading byte " << i << ": " << line << endl;      
					IMem[i] = bitset<8>(line);
					i++;
				}                    
			}
            else cout<<"Unable to open IMEM input file.";
			imem.close();                     
		}

		bitset<32> readInstr(bitset<32> ReadAddress) {    
			int addr = ReadAddress.to_ulong();
            bitset<32> instr; // To hold the final instruction

            // Loop to read four bytes (little-endian format)
            for (int i = 0; i < 4; ++i) {
                instr <<= 8; // Shift left by 8 bits to make space for the next byte
                instr |= IMem[addr + i].to_ulong(); // Read each byte in little-endian order
            }
            
             cout << "Fetched instruction at address " << addr << ": 0x" 
         << hex << instr.to_ulong() << dec << endl;
            return instr;      
        }
    private:
        vector<bitset<8> > IMem;     
};
      
class DataMem    
{
    public: 
		string id, opFilePath, ioDir;
        DataMem(string name, string ioDir) : id{name}, ioDir{ioDir} {
            DMem.resize(MemSize);
			opFilePath = ioDir + "\\"+name + "_DMEMResult.txt";
            ifstream dmem;
            string line;
            int i=0;
            dmem.open(ioDir + "\\dmem.txt");
            if (dmem.is_open())
            {
                while (getline(dmem,line))
                {      
                    DMem[i] = bitset<8>(line);
                    i++;
                }
            }
            else cout<<"Unable to open DMEM input file.";
                dmem.close();          
        }
		
        bitset<32> readDataMem(bitset<32> Address) {	
			int addr = Address.to_ulong();
            bitset<32> data;
    
            for (int i = 0; i < 4; i++) {
                data <<= 8;
                data |= bitset<32>(DMem[addr + i].to_ulong());
            }
    
            return data;
		}
            
        void writeDataMem(bitset<32> Address, bitset<32> WriteData) {
			int addr = Address.to_ulong();
    
            for (int i = 3; i >= 0; i--) {
                DMem[addr + i] = bitset<8>(WriteData.to_ulong() & 0xFF);
                WriteData >>= 8;
            }
        }   
                     
        void outputDataMem() {
            ofstream dmemout;
            dmemout.open(opFilePath, std::ios_base::trunc);
            if (dmemout.is_open()) {
                for (int j = 0; j< 1000; j++)
                {     
                    dmemout << DMem[j]<<endl;
                }
                     
            }
            else cout<<"Unable to open "<<id<<" DMEM result file." << endl;
            dmemout.close();
        }             

    private:
		vector<bitset<8> > DMem;      
};

class RegisterFile
{
    public:
		string outputFile;
     	RegisterFile(string ioDir, string identifier) {
            outputFile.clear();
			outputFile = ioDir + "\\"+ identifier + "RFResult.txt";
            cout << "Initializing RegisterFile with path: " << outputFile << endl;
			Registers.resize(32);  
			Registers[0] = bitset<32> (0);
        }
	
        bitset<32> readRF(bitset<5> Reg_addr) {  

            cout << "Reading register at address: " << Reg_addr.to_ulong() 
            << " with value: " << Registers[Reg_addr.to_ulong()] << endl;

            return Registers[Reg_addr.to_ulong()];

            

        }
    
        void writeRF(bitset<5> Reg_addr, bitset<32> Wrt_reg_data) {
            if (Reg_addr.to_ulong() != 0)
            {  // Register 0 is always hard-wired to zero
                Registers[Reg_addr.to_ulong()] = Wrt_reg_data;
            }
        }
		 
		void outputRF(int cycle) {
			ofstream rfout;
			if (cycle == 0)
				rfout.open(outputFile, std::ios_base::trunc);
			else 
				rfout.open(outputFile, std::ios_base::app);
			if (rfout.is_open())
			{
				rfout<<"State of RF after executing cycle:\t"<<cycle<<endl;
				for (int j = 0; j<32; j++)
				{
					rfout << Registers[j]<<endl;
				}
				
			}
			else cout<<"Unable to open RF output file."<<endl;
			rfout.close();               
		} 
			
	private:
		vector<bitset<32> >Registers;
};

class Core {
	public:
		RegisterFile myRF;
		uint32_t cycle = 0;
		bool halted = false;
		string ioDir;
		struct stateStruct state, nextState;
		InsMem ext_imem;
		DataMem ext_dmem;
		
		Core(string ioDir, string identifier, InsMem &imem, DataMem &dmem): myRF(ioDir, identifier), ioDir{ioDir}, ext_imem {imem}, ext_dmem {dmem} {}

		virtual void step() {}

		virtual void printState() {}
};

class SingleStageCore : public Core {
public:
    SingleStageCore(string ioDir, InsMem &imem, DataMem &dmem) 
        : Core(ioDir, "SS_", imem, dmem), opFilePath(ioDir + "\\StateResult_SS.txt") {
        state.IF.PC = 0; // Initialize PC to 0
        state.IF.nop = false;
        numCycles = 0;
        numInstructions = 0;
        performanceFilePath = ioDir + "\\PerformanceMetrics_Result.txt";
    }

    void step() {

        numCycles++;

        if (state.IF.nop) {
        halted = true;
        return;
    }

        cout << "Reading instruction at PC: " << state.IF.PC.to_ulong() << endl;
        state.ID.Instr = ext_imem.readInstr(state.IF.PC);
        cout << "Instruction read: " << state.ID.Instr << endl;
    
    
    if (state.ID.Instr.to_ulong() == 0xFFFFFFFF) {
        cout << "Halt instruction detected" << endl;
        state.IF.nop = true;
        halted = true;
        // Print final state before returning
        myRF.outputRF(cycle);
        printState(state, cycle);
        cycle++;
        return;
    }

    if (state.ID.Instr.to_ulong() == 0x00000013) {  // NOP instruction
        cout << "NOP instruction detected. Skipping this cycle." << endl;
        state.IF.PC = state.IF.PC.to_ulong() + 4;  // Advance PC by 4 bytes
        printState(state, cycle);
        cycle++;
        return;
    }

    numInstructions++;

    /* -------- ID stage -------- */
    // Extract instruction fields
    uint32_t instr = state.ID.Instr.to_ulong();
    bitset<7> opcode(instr & 0x7F);
    bitset<5> rd((instr >> 7) & 0x1F);
    bitset<3> funct3((instr >> 12) & 0x7);
    bitset<5> rs1((instr >> 15) & 0x1F);
    bitset<5> rs2((instr >> 20) & 0x1F);
    bitset<7> funct7((instr >> 25) & 0x7F);

    cout << "Reading from registers - rs1: " << rs1.to_ulong() 
         << " rs2: " << rs2.to_ulong() << endl;

    // Read register values
    bitset<32> rs1_val = myRF.readRF(rs1);
    bitset<32> rs2_val = myRF.readRF(rs2);

    cout << "Register values - rs1_val: " << rs1_val.to_string() 
         << " rs2_val: " << rs2_val.to_string() << endl;

    /* -------- EX stage -------- */
    bitset<32> result;
    bool write_reg = false;
    bool branch_taken = false;
    uint32_t next_pc = state.IF.PC.to_ulong() + 4;

    // R-type instructions
    if (opcode == 0b0110011) {
        write_reg = true;
        cout << "rs1_val: " << rs1_val << ", rs2_val: " << rs2_val << endl;

        if (funct3 == 0b000) {  // ADD/SUB
            if (funct7 == 0b0000000) {  // ADD
                result = bitset<32>(rs1_val.to_ulong() + rs2_val.to_ulong());
                cout << "result (ADD): " << result << endl;
            } else if (funct7 == 0b0100000) {  // SUB
                result = bitset<32>(rs1_val.to_ulong() - rs2_val.to_ulong());
                cout << "result (SUB): " << result << endl;
            }
        

        }
        else if (funct3 == 0b110) {  // OR
                result = rs1_val | rs2_val;
                cout << "OR result: " << result << endl;
            }
        else if (funct3 == 0b111) {  // AND
                result = rs1_val & rs2_val;
                cout << "AND result: " << result << endl;
            }
        else if (funct3 == 0b100) {  // XOR
                result = rs1_val ^ rs2_val;
                cout << "XOR result: " << result << endl;
            }
        }

    if (opcode == 0b1100011) {
            int32_t imm = ((instr >> 31) & 0x1) << 12;  
            imm |= ((instr >> 7) & 0x1) << 11;          
            imm |= ((instr >> 25) & 0x3F) << 5;         
            imm |= ((instr >> 8) & 0xF) << 1;           
            
            // Sign extend
            if (imm & 0x1000) imm |= 0xFFFFE000;
            
            if (funct3 == 0b000) {  // BEQ
                if (rs1_val == rs2_val) {
                    next_pc = state.IF.PC.to_ulong() + imm;
                    branch_taken = true;
                }
            }
            else if (funct3 == 0b001) {  // BNE
                if (rs1_val != rs2_val) {
                    next_pc = state.IF.PC.to_ulong() + imm;
                    branch_taken = true;
                }
            }
        }
        // JAL instruction
        else if (opcode == 0b1101111) {  // JAL
            // Calculate immediate for JAL
            int32_t imm = ((instr >> 31) & 0x1) << 20;  
            imm |= ((instr >> 12) & 0xFF) << 12;        
            imm |= ((instr >> 20) & 0x1) << 11;         
            imm |= ((instr >> 21) & 0x3FF) << 1;        
            
            // Sign extend
            if (imm & 0x100000) imm |= 0xFFE00000;
            
            // Store return address (PC + 4)
            result = bitset<32>(state.IF.PC.to_ulong() + 4);
            write_reg = true;
            
            // Calculate target address
            next_pc = state.IF.PC.to_ulong() + imm;
            branch_taken = true;
        }

    // Load instructions
    else if (opcode == 0b0000011) {
        if (funct3 == 0b010) {  // LW
            int32_t imm = ((instr >> 20) & 0xFFF);
            // Sign extend
            if (imm & 0x800) imm |= 0xFFFFF000;
            uint32_t addr = rs1_val.to_ulong() + imm;
            result = ext_dmem.readDataMem(bitset<32>(addr));
            write_reg = true;
        }
        else if (funct3 == 0b000) {  // LB
            int32_t imm = ((instr >> 20) & 0xFFF);
        // Sign extend immediate
            if (imm & 0x800) imm |= 0xFFFFF000;
            uint32_t addr = rs1_val.to_ulong() + imm;
        
        // Read the full word from memory
            bitset<32> word = ext_dmem.readDataMem(bitset<32>(addr));
        
        // Extract the byte based on the address
            int byte_offset = addr & 0x3;  // Get last 2 bits for byte position
            uint8_t byte = (word.to_ulong() >> (byte_offset * 8)) & 0xFF;
        
        // Sign extend the byte to 32 bits
            int32_t signed_byte = static_cast<int8_t>(byte);
            result = bitset<32>(signed_byte);
        
            write_reg = true;
    }
    }

  

    else if (opcode == 0b0010011) {
            write_reg = true;
            // Extract and sign-extend immediate
            int32_t imm = ((instr >> 20) & 0xFFF);
            // Sign extend if negative
            if (imm & 0x800) imm |= 0xFFFFF000;
            
            bitset<32> imm_val(imm);
            cout << "I-type immediate value: " << imm_val << endl;

            if (funct3 == 0b000) {  // ADDI
                result = bitset<32>(rs1_val.to_ulong() + imm);
                cout << "ADDI result: " << result << endl;
            }
            else if (funct3 == 0b100) {  // XORI
                result = rs1_val ^ imm_val;
                cout << "XORI result: " << result << endl;
            }
            else if (funct3 == 0b110) {  // ORI
                result = rs1_val | imm_val;
                cout << "ORI result: " << result << endl;
            }
            else if (funct3 == 0b111) {  // ANDI
                result = rs1_val & imm_val;
                cout << "ANDI result: " << result << endl;
            }
        }

    // Store instructions
    else if (opcode == 0b0100011) {
        if (funct3 == 0b010) {  // SW
            int32_t imm = ((instr >> 25) & 0xFE0) | ((instr >> 7) & 0x1F);
            // Sign extend
            if (imm & 0x800) imm |= 0xFFFFF000;
            uint32_t addr = rs1_val.to_ulong() + imm;
            ext_dmem.writeDataMem(bitset<32>(addr), rs2_val);
        }
    }

    if (state.EX.wrt_mem) {
            ext_dmem.writeDataMem(state.EX.ALUresult, state.EX.Read_data2);
            cout << "Writing memory at address: " << state.EX.ALUresult.to_ulong() << endl;
        }
        if (state.EX.rd_mem) {
            state.MEM.ALUresult = ext_dmem.readDataMem(state.EX.ALUresult);
            cout << "Reading memory at address: " << state.EX.ALUresult.to_ulong() << endl;
            state.MEM.Store_data = state.EX.Read_data2;
            state.MEM.Rs = state.EX.Rs;
            state.MEM.Rt = state.EX.Rt;
            state.MEM.Wrt_reg_addr = state.EX.Wrt_reg_addr;
            state.MEM.rd_mem = state.EX.rd_mem;
            state.MEM.wrt_mem = state.EX.wrt_mem;
            state.MEM.wrt_enable = state.EX.wrt_enable;
            state.MEM.nop = state.EX.nop;
        } 

    /* -------- WB stage -------- */
    if (write_reg && rd.to_ulong() != 0) {
        myRF.writeRF(rd, result);
    }

    /* -------- PC Update -------- */
    state.IF.PC = bitset<32>(next_pc);

    /* -------- State Update -------- */
    myRF.outputRF(cycle);
    printState(state, cycle);
    nextState = state;
    cycle++;
    }

    void printState(stateStruct state, int cycle) {
        ofstream printstate;
        if (cycle == 0)
            printstate.open(opFilePath, std::ios_base::trunc);
        else 
            printstate.open(opFilePath, std::ios_base::app);
        if (printstate.is_open()) {
            printstate << "State after executing cycle:\t" << cycle << endl;
            printstate << "IF.PC:\t" << state.IF.PC.to_ulong() << endl;
            printstate << "IF.nop:\t" << (state.IF.nop ? "true" : "false") << endl;
        }
        else cout << "Unable to open SS StateResult output file." << endl;
        printstate.close();
    }

    void outputPerformanceMetrics() {
        ofstream perfout(performanceFilePath, std::ios_base::trunc);
        if (perfout.is_open()) {
            double CPI = (numInstructions > 0) ? static_cast<double>(numCycles) / numInstructions : 0;
            double IPC = (numCycles > 0) ? static_cast<double>(numInstructions) / numCycles : 0;
            
            perfout << "Single Stage Core Performance Metrics\n";
            perfout << "Number of cycles taken: " << numCycles << endl;
            perfout << "Total Number of Instructions: " << numInstructions << endl;
            perfout << "Cycles per instruction: " << fixed << setprecision(2) << CPI << endl;
            perfout << "Instructions per cycle: " << fixed << setprecision(2) << IPC << endl;
        } else {
            cout << "Unable to open PerformanceMetrics_Result file." << endl;
        }
        perfout.close();
    }
    ~SingleStageCore() {
        outputPerformanceMetrics();  
    }

private:
    string opFilePath;
    string performanceFilePath;
    int numCycles;
    int numInstructions;
};


class FiveStageCore : public Core{
	public:
		
		FiveStageCore(string ioDir, InsMem &imem, DataMem &dmem): Core(ioDir, "FS_", imem, dmem), opFilePath(ioDir + "StateResult_FS.txt") {}

		void step() {
			
			/* --------------------- WB stage --------------------- */
			
			
			
			/* --------------------- MEM stage -------------------- */
			
			
			
			/* --------------------- EX stage --------------------- */
			
			
			
			/* --------------------- ID stage --------------------- */
			
			
			
			/* --------------------- IF stage --------------------- */
			
			
			halted = true;
			if (state.IF.nop && state.ID.nop && state.EX.nop && state.MEM.nop && state.WB.nop)
				halted = true;
        
            myRF.outputRF(cycle); // dump RF
			printState(nextState, cycle); //print states after executing cycle 0, cycle 1, cycle 2 ... 
       
			state = nextState; //The end of the cycle and updates the current state with the values calculated in this cycle
			cycle++;
		}

		void printState(stateStruct state, int cycle) {
		    ofstream printstate;
			if (cycle == 0)
				printstate.open(opFilePath, std::ios_base::trunc);
			else 
		    	printstate.open(opFilePath, std::ios_base::app);
		    if (printstate.is_open()) {
		        printstate<<"State after executing cycle:\t"<<cycle<<endl; 

		        printstate<<"IF.PC:\t"<<state.IF.PC.to_ulong()<<endl;        
		        printstate<<"IF.nop:\t"<<state.IF.nop<<endl; 

		        printstate<<"ID.Instr:\t"<<state.ID.Instr<<endl; 
		        printstate<<"ID.nop:\t"<<state.ID.nop<<endl;

		        printstate<<"EX.Read_data1:\t"<<state.EX.Read_data1<<endl;
		        printstate<<"EX.Read_data2:\t"<<state.EX.Read_data2<<endl;
		        printstate<<"EX.Imm:\t"<<state.EX.Imm<<endl; 
		        printstate<<"EX.Rs:\t"<<state.EX.Rs<<endl;
		        printstate<<"EX.Rt:\t"<<state.EX.Rt<<endl;
		        printstate<<"EX.Wrt_reg_addr:\t"<<state.EX.Wrt_reg_addr<<endl;
		        printstate<<"EX.is_I_type:\t"<<state.EX.is_I_type<<endl; 
		        printstate<<"EX.rd_mem:\t"<<state.EX.rd_mem<<endl;
		        printstate<<"EX.wrt_mem:\t"<<state.EX.wrt_mem<<endl;        
		        printstate<<"EX.alu_op:\t"<<state.EX.alu_op<<endl;
		        printstate<<"EX.wrt_enable:\t"<<state.EX.wrt_enable<<endl;
		        printstate<<"EX.nop:\t"<<state.EX.nop<<endl;        

		        printstate<<"MEM.ALUresult:\t"<<state.MEM.ALUresult<<endl;
		        printstate<<"MEM.Store_data:\t"<<state.MEM.Store_data<<endl; 
		        printstate<<"MEM.Rs:\t"<<state.MEM.Rs<<endl;
		        printstate<<"MEM.Rt:\t"<<state.MEM.Rt<<endl;   
		        printstate<<"MEM.Wrt_reg_addr:\t"<<state.MEM.Wrt_reg_addr<<endl;              
		        printstate<<"MEM.rd_mem:\t"<<state.MEM.rd_mem<<endl;
		        printstate<<"MEM.wrt_mem:\t"<<state.MEM.wrt_mem<<endl; 
		        printstate<<"MEM.wrt_enable:\t"<<state.MEM.wrt_enable<<endl;         
		        printstate<<"MEM.nop:\t"<<state.MEM.nop<<endl;        

		        printstate<<"WB.Wrt_data:\t"<<state.WB.Wrt_data<<endl;
		        printstate<<"WB.Rs:\t"<<state.WB.Rs<<endl;
		        printstate<<"WB.Rt:\t"<<state.WB.Rt<<endl;
		        printstate<<"WB.Wrt_reg_addr:\t"<<state.WB.Wrt_reg_addr<<endl;
		        printstate<<"WB.wrt_enable:\t"<<state.WB.wrt_enable<<endl;
		        printstate<<"WB.nop:\t"<<state.WB.nop<<endl; 
		    }
		    else cout<<"Unable to open FS StateResult output file." << endl;
		    printstate.close();
		}
	private:
		string opFilePath;
};

int main(int argc, char* argv[]) {
	
	string ioDir = "";
    if (argc == 1) {
        cout << "Enter path containing the memory files: ";
        cin >> ioDir;
        ifstream test(ioDir + "\\imem.txt");
    if (!test.is_open()) {
        cout << "Cannot open imem.txt file! Please check if path is correct." << endl;
        cout << "Input directory: " << ioDir << endl;
        return -1;
}


test.close();
    }
    else if (argc > 2) {
        cout << "Invalid number of arguments. Machine stopped." << endl;
        return -1;
    }
    else {
        ioDir = argv[1];
        cout << "IO Directory: " << ioDir << endl;
    }

    InsMem imem = InsMem("Imem", ioDir);
    DataMem dmem_ss = DataMem("SS", ioDir);
	DataMem dmem_fs = DataMem("FS", ioDir);

	SingleStageCore SSCore(ioDir, imem, dmem_ss);
	FiveStageCore FSCore(ioDir, imem, dmem_fs);

    while (1) {
		if (!SSCore.halted)
			SSCore.step();
		
		if (!FSCore.halted)
			FSCore.step();

		if (SSCore.halted && FSCore.halted)
			break;
    }
    
	// dump SS and FS data mem.
	dmem_ss.outputDataMem();
	dmem_fs.outputDataMem();

	return 0;
}