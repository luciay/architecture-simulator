#include<iostream>
#include<string>
#include<vector>
#include<bitset>
#include<fstream>
using namespace std;
#define ADDU 1
#define SUBU 3
#define AND 4
#define OR  5
#define NOR 7
#define MemSize 65536

//register file
class RF
{
public:
    bitset<32> ReadData1, ReadData2;
    RF()
    {
        Registers.resize(32);
        Registers[0] = bitset<32> (0);
    }
    
    void ReadWrite(bitset<5> RdReg1, bitset<5> RdReg2, bitset<5> WrtReg, bitset<32> WrtData, bitset<1> WrtEnable)
    {
        //if WrtEnable used, then use given address to register to write the data back
        if (WrtEnable == 1)
        {
            Registers[WrtReg.to_ulong()] = WrtData;
        }
        
        //else just use read registers
        else
        {
            ReadData1 = Registers[RdReg1.to_ulong()];
            ReadData2 = Registers[RdReg2.to_ulong()];
        }
    }
    
    void OutputRF()
    {
        ofstream rfout;
        rfout.open("RFresult.txt",std::ios_base::app);
        if (rfout.is_open())
        {
            rfout<<"A state of RF:"<<endl;
            for (int j = 0; j<32; j++)
            {
                rfout << Registers[j]<<endl;
            }
            
        }
        else cout<<"Unable to open file";
        rfout.close();
        
    }
private:
    vector<bitset<32> >Registers;
    
};

//ALU
class ALU
{
public:
    bitset<32> ALUresult;
    bitset<32> ALUOperation (bitset<3> ALUOP, bitset<32> oprand1, bitset<32> oprand2)
    {
        unsigned long result;
        
        int aluCase = ALUOP.to_ulong();
        
        switch (aluCase) {
                //ADDU and ADDIU
            case 1:
            {
                result = oprand1.to_ulong() + oprand2.to_ulong();
                ALUresult = bitset<32>(result);
                break;
            }
                
                //SUBU
            case 3:
            {
                result = oprand1.to_ulong() - oprand2.to_ulong();
                ALUresult = bitset<32>(result);
                break;
            }
                //AND
            case 4:
                ALUresult = oprand1 & oprand2;
                break;
                //OR
            case 5:
                ALUresult = oprand1 | oprand2;
                break;
                //BEQ
                //NOR
            case 7:
                ALUresult = ~(oprand1 | oprand2);
                break;
                
            default:
                break;
        }
        return ALUresult;
    }
};

//instruction memory
class INSMem
{
public:
    bitset<32> Instruction;
    INSMem()
    {       IMem.resize(MemSize);
        ifstream imem;
        string line;
        int i=0;
        imem.open("imem.txt");
        if (imem.is_open())
        {
            while (getline(imem,line))
            {
                IMem[i] = bitset<8>(line);
                i++;
            }
        }
        else cout<<"Unable to open file";
        imem.close();
        
    }
    
    bitset<32> ReadMemory (bitset<32> ReadAddress)
    {
        //Read the byte at the ReadAddress and the following three byte
        string instr = "", temp;
        
        //for loop: starting at the given address and reading the next four lines, concatenate each byte (string)
        for (int i = 0; i <= 3; i++)
        {
            temp = (IMem[ReadAddress.to_ulong() + i]).to_string();
            instr += temp;
        }
        
        //convert concatenated string to bitset32-type
        Instruction = bitset<32>(instr);
        return Instruction;
    }
    
private:
    vector<bitset<8> > IMem;
    
};

//data memory
class DataMem
{
public:
    bitset<32> readdata;
    DataMem()
    {
        DMem.resize(MemSize);
        ifstream dmem;
        string line;
        int i=0;
        dmem.open("dmem.txt");
        if (dmem.is_open())
        {
            while (getline(dmem,line))
            {
                DMem[i] = bitset<8>(line);
                i++;
            }
            cout << endl;
        }
        else cout<<"Unable to open file";
        dmem.close();
        
    }
    bitset<32> MemoryAccess (bitset<32> Address, bitset<32> WriteData, bitset<1> readmem, bitset<1> writemem)
    {
        bitset<32> readdata;
        
        //if writemem == 1
        if (writemem == 1 && readmem == 0)
        {
            //take 32bits and convert to string type to use substr
            string writeStr = WriteData.to_string();
            
            for (int z = 0, i = 0; z<=3; i += 8, z++)
            {
                //write to each line starting at the Address given
                //take substr(0, 8) and place into DMem
                DMem[Address.to_ulong() + z] = bitset<8>(writeStr.substr(i, 8));
            }
            readdata = 0;
        }
        
        //if readmem == 1
        if (writemem == 0 && readmem == 1)
        {
            string instr = "", temp;
            
            //for loop: starting at the given address and reading the next four lines, concatenate each byte (string)
            for (int i = 0; i <= 3; i++)
            {
                temp = (DMem[Address.to_ulong() + i]).to_string();
                instr += temp;
            }
            readdata = bitset<32>(instr);
        }
        
        return readdata;
    }
    
    void OutputDataMem()
    {
        ofstream dmemout;
        dmemout.open("dmemresult.txt");
        if (dmemout.is_open())
        {
            for (int j = 0; j< 1000; j++)
            {
                dmemout << DMem[j]<<endl;
            }
            
        }
        else cout<<"Unable to open file";
        dmemout.close();
        
    }
    
private:
    vector<bitset<8> > DMem;
    
};

// here we decode the opcode and process the instruction appropriately
int main()
{
    RF myRF;
    ALU myALU;
    INSMem myInsMem;
    DataMem myDataMem;
    
    // initialize program counter at the very start
    // default constructor for bitset initializes all values to 0
    bitset<32> PC;
    
    while (1)
    {
        //boolean flag for if PC has been encountered
        bool pcFlag = 0;
        
        //*******INSTRUCTION FETCH*******
        //this gives the returned instruction located at the PC value (which was initialized to 0)
        bitset<32> currInstr = myInsMem.ReadMemory(PC);
        
        
        //deriving OpCode from the instruction:
        
        //convert bitset to string
        string stringInstr = currInstr.to_string();
        
        //decode OpCode
        string opcode = stringInstr.substr(0, 6);
        
        
        //Categorize OpCodes
        
        //OpCode = 00 OR 09: R-TYPE or ADDIU
        if (opcode == "000000" || opcode == "001001")
        {
            //if RTYPE
            if (opcode == "000000")
            {
                //setting the variables
                bitset<32> WrtData;
                bitset<5> RdReg1, RdReg2, WrtReg;
                bitset<3> function = bitset<3>(stringInstr.substr(29, 3)); //function = stringInstr.substr(26, 31);
                bitset<1> WrtEnable;
                
                RdReg1 = bitset<5>(stringInstr.substr(6, 5));
                RdReg2 = bitset<5>(stringInstr.substr(11, 5));
                WrtReg = bitset<5>(stringInstr.substr(16, 5));
                
                //READ from registers; set WrtEnable to 0
                WrtEnable = bitset<1>(0);
                myRF.ReadWrite(RdReg1, RdReg2, WrtReg, WrtData, WrtEnable);
                
                //call the ALU
                WrtData = myALU.ALUOperation(function, myRF.ReadData1, myRF.ReadData2);
                
                //WRITE to registers; set WrtEnable to 1
                WrtEnable = bitset<1>(1);
                myRF.ReadWrite(RdReg1, RdReg2, WrtReg, WrtData, WrtEnable);
            }
            
            //else if ITYPE
            else
            {
                bitset<32> WrtData, imm;
                bitset<5> RdReg1, RdReg2, WrtReg;
                bitset<1> WrtEnable;
                
                RdReg1 = bitset<5>(stringInstr.substr(6, 5));
                WrtReg = bitset<5>(stringInstr.substr(11, 5));    //WrtReg is the now-former RdReg2 substr
                
                //check if immediate value is negative
                imm = bitset<32>(stringInstr.substr(16, 16));       //immediate values is in remaining string
                
                
                //READ
                WrtEnable = bitset<1>(0);
                myRF.ReadWrite(RdReg1, RdReg2, WrtReg, WrtData, WrtEnable);
                
                //call the ALU
                bitset<3> function(1);    //this is the ADDU case in ALU
                WrtData = myALU.ALUOperation(function, myRF.ReadData1, imm);    //adding immediate values, not myRF.ReadData2
                
                //WRITE to registers; set WrtEnable to 1
                WrtEnable = bitset<1>(1);
                myRF.ReadWrite(RdReg1, RdReg2, WrtReg, WrtData, WrtEnable);
            }
        }
        
        //OpCode = 23 OR 2B: LW, SW - ACCESS DMEM
        else if (opcode == "100011" || opcode == "101011")
        {
            if (opcode == "100011") //load word
            {
                // R[rt] = M[ R[rs] + imm ]
                bitset<32> WrtData, imm;
                bitset<5> RdReg1, RdReg2, WrtReg;
                bitset<1> WrtEnable;
                
                RdReg1 = bitset<5>(stringInstr.substr(6, 5));
                WrtReg = bitset<5>(stringInstr.substr(11, 5));    //WrtReg is the now-former RdReg2 substr
                imm = bitset<32>(stringInstr.substr(16, 16));       //immediate values is in remaining string
                
                //READ
                WrtEnable = bitset<1>(0);
                myRF.ReadWrite(RdReg1, RdReg2, WrtReg, WrtData, WrtEnable);
                
                //call the ALU
                bitset<3> function(1);                                                //this is the ADDU case in ALU
                bitset<32> Address = myALU.ALUOperation(function, myRF.ReadData1, imm); //adding immediate values, not myRF.ReadData2
                
                //LOAD FROM MEM
                //myDataMem.MemoryAccess(bitset<32> Address, bitset<32> WriteData, bitset<1> readmem, bitset<1> writemem);
                WrtData = myDataMem.MemoryAccess(Address, 0, 1, 0);
                
                //WRITE back to register
                WrtEnable = bitset<1>(1);
                myRF.ReadWrite(RdReg1, RdReg2, WrtReg, WrtData, WrtEnable);
            }
            
            else                    //store word
            {
                // M[ R[rs] + imm ] = R[rt]
                bitset<32> WrtData, imm;
                bitset<5> RdReg1, RdReg2, WrtReg;
                bitset<1> WrtEnable;
                
                RdReg1 = bitset<5>(stringInstr.substr(6, 5));     //R[rs]
                RdReg2 = bitset<5>(stringInstr.substr(11, 5));    //R[rt]
                imm = bitset<32>(stringInstr.substr(16, 16));       //immediate values is in remaining string
                
                //READ
                WrtEnable = bitset<1>(0);
                myRF.ReadWrite(RdReg1, RdReg2, WrtReg, WrtData, WrtEnable);
                
                //call the ALU
                bitset<3> function(1);                                                //this is the ADDU case in ALU
                bitset<32> Address = myALU.ALUOperation(function, myRF.ReadData1, imm); //adding immediate values, not myRF.ReadData2
                
                //WRITE TO MEM
                //myDataMem.MemoryAccess(bitset<32> Address, bitset<32> WriteData, bitset<1> readmem, bitset<1> writemem);
                //myDataMem.MemoryAccess(R[rs] + imm, R[rt], read = 0, write = 1);
                WrtData = myDataMem.MemoryAccess(Address, myRF.ReadData2, 0, 1);
            }
        }
        
        //OpCode = 02 OR 04: J OR BEQ
        else if (opcode == "000010" || opcode == "000100")
        {
            if (opcode == "000010")
            {
                PC = myALU.ALUOperation(1, PC, 4);  //PC + 4
                string JPCbits = (PC.to_string()).substr(0, 4); //take four MSB
                
                
                bitset<32> jAddressbitset = bitset<32>(JPCbits+stringInstr.substr(6, 26)+"00"); //concatenate address
                PC = jAddressbitset;
                
                //boolean flag will not update PC again
                pcFlag = 1;
            }
            else
            {
                //setting the variables
                bitset<32> WrtData;
                bitset<32>imm;
                bitset<5> RdReg1, RdReg2, WrtReg;
                bitset<1> WrtEnable;
                
                RdReg1 = bitset<5>(stringInstr.substr(6, 5));     //R[rs]
                RdReg2 = bitset<5>(stringInstr.substr(11, 5));    //R[rt]
                imm = bitset<32>(stringInstr.substr(16, 16));    //imm
                
                //READ from registers; set WrtEnable to 0
                WrtEnable = bitset<1>(0);
                myRF.ReadWrite(RdReg1, RdReg2, WrtReg, WrtData, WrtEnable);
                
                //call the ALU
                bitset<3> function = 3; //this is the case for SUBU
                WrtData = myALU.ALUOperation(function, myRF.ReadData1, myRF.ReadData2);
                
                //if rs == rt
                if (WrtData == bitset<32>(0))
                {
                    bitset<3> function = bitset<3>(1); //this is the case for ADDU
                    PC = myALU.ALUOperation(function, PC, imm);
                }
                
                //else, do nothing
            }
        }
        
        
        
        //OpCode = 3F, HALT
        else if (opcode == "111111")
            break;
        
        //PC = PC + 4
        if (pcFlag == 0)
            PC = myALU.ALUOperation(1, PC, 4);
        
        myRF.OutputRF(); // dump RF;
    }
    myDataMem.OutputDataMem(); // dump data mem
    
    return 0;
    
}
