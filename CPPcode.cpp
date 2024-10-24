#include<iostream>
#include<string>
#include<vector>
#include<bitset>
#include<fstream>
#include<cstdint>

using namespace std;

#define MemSize 1000 // memory size, in reality, the memory size should be 2^32, but for this lab, for the space resaon, we keep it as this large number, but the memory is still 32-bit addressable.

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
			imem.open(ioDir + "\\input\\imem.txt");
			if (imem.is_open())
			{
				while (getline(imem,line))
				{      
					IMem[i] = bitset<8>(line);
					i++;
				}                    
			}
            else cout<<"Unable to open IMEM input file.";
			imem.close();                     
		}

		bitset<32> readInstr(bitset<32> ReadAddress) {    
			int addr = (int)(ReadAddress.to_ulong());
            bitset<32> instruction;
    
    // Combine four 8-bit chunks from memory to form a 32-bit instruction
            for (int i = 0; i < 4; i++) 
            {
                instruction <<= 8;
                instruction |= bitset<32>(IMem[addr + i].to_ulong());
            }

            return instruction;
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
			opFilePath = ioDir + "\\outputs\\" + name + "_DMEMResult.txt";
            ifstream dmem;
            string line;
            int i=0;
            dmem.open(ioDir + "\\input\\dmem.txt");
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
			outputFile = ioDir + "\\outputs\\"+ identifier + "RFResult.txt";
			Registers.resize(32);  
			Registers[0] = bitset<32> (0);  
        }
	
        bitset<32> readRF(bitset<5> Reg_addr) {   
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
        : Core(ioDir, "SS_", imem, dmem), opFilePath(ioDir + "\\outputs\\StateResult_SS.txt") {
        state.IF.PC = 0; // Initialize PC to 0
        state.IF.nop = false;
    }

    void step() {
        // Fetch instruction
        state.ID.Instr = ext_imem.readInstr(state.IF.PC);
        cout << "Cycle " << cycle << ": Fetched Instruction: " << state.ID.Instr << endl;
        if (state.ID.Instr.to_ulong() == 0xFFFFFFFF) { // NOP or HALT condition
            state.IF.nop = true;
            halted = true;
        } else {
            // Decode instruction
            bitset<7> opcode = bitset<7>((state.ID.Instr.to_ulong() & 0x7F)); // Extract opcode (bits 0-6)
            bitset<5> rd = bitset<5>((state.ID.Instr.to_ulong() >> 7) & 0x1F); // Extract rd (bits 7-11)
            bitset<3> funct3 = bitset<3>((state.ID.Instr.to_ulong() >> 12) & 0x7); // Extract funct3 (bits 12-14)
            bitset<5> rs1 = bitset<5>((state.ID.Instr.to_ulong() >> 15) & 0x1F); // Extract rs1 (bits 15-19)
            bitset<5> rs2 = bitset<5>((state.ID.Instr.to_ulong() >> 20) & 0x1F); // Extract rs2 (bits 20-24)
            bitset<7> funct7 = bitset<7>((state.ID.Instr.to_ulong() >> 25) & 0x7F); // Extract funct7 (bits 25-31)
            
			 cout << "Decoded Fields - Opcode: " << opcode << ", rd: " << rd << ", funct3: " << funct3
             << ", rs1: " << rs1 << ", rs2: " << rs2 << ", funct7: " << funct7 << endl;
            // Check if this is an ADD instruction (opcode = 0x33, funct3 = 0x0, funct7 = 0x00)
            if (opcode == 0x33 && funct3 == 0x0 && funct7 == 0x00) {
                // ADD operation: Rd = Rs1 + Rs2
                bitset<32> val1 = myRF.readRF(rs1); // Read register Rs1
                bitset<32> val2 = myRF.readRF(rs2); // Read register Rs2

				cout << "Cycle " << cycle << ": ADD operation - rs1 value: " << val1.to_ulong()
                 << ", rs2 value: " << val2.to_ulong() << endl;

                bitset<32> result = bitset<32>(val1.to_ulong() + val2.to_ulong()); // Perform addition

                // Write result to destination register (Rd)
                myRF.writeRF(rd, result);

				cout << "Writing to register rd: " << rd.to_ulong() << " with value: " << result.to_ulong() << endl;
            }
            
            // Increment PC
            state.IF.PC = bitset<32>(state.IF.PC.to_ulong() + 4);
        }

        // Dump RF and print the state for this cycle
        myRF.outputRF(cycle);
        printState(nextState, cycle);

        // Update states for the next cycle
        state = nextState;
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
            printstate << "IF.nop:\t" << state.IF.nop << endl;
        }
        else cout << "Unable to open SS StateResult output file." << endl;
        printstate.close();
    }

private:
    string opFilePath;
};


class FiveStageCore : public Core{
	public:
		
		FiveStageCore(string ioDir, InsMem &imem, DataMem &dmem): Core(ioDir, "FS_", imem, dmem), opFilePath(ioDir + "\\outputs\\StateResult_FS.txt") {}

		void step() {
			/* Your implementation */
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