// Members: Jacob Cordonero, 
//          Juan Guarnizo Lopez,
//          Poojan Yagnik

#include "spimcore.h"

/* ALU: 10 points by Jacob */
void ALU(unsigned A, unsigned B, char ALUControl, unsigned *ALUresult, char *Zero){

    if(ALUControl == 0x0) {
        *ALUresult = A + B;
    } 
    else if (ALUControl == 0x1) {
        *ALUresult = A - B;
    } 
    else if (ALUControl == 0x2) {
        // if A < B, Z = 1; otherwise, Z = 0 
        int a = A;
        int b = B;
        *ALUresult = a < b;
    } 
    // if A < B, Z = 1; otherwise, Z = 0 (A and B are unsigned integers)
    else if (ALUControl == 0x3){
        *ALUresult = A < B;
    } 
    else if (ALUControl == 0x4){
        *ALUresult = A & B;
    } 
    else if (ALUControl == 0x5){
        *ALUresult = A | B;
    } 
    else if (ALUControl == 0x6){
        *ALUresult = B << 16;
    } 
    else if (ALUControl == 0x7){
        *ALUresult = ~A;
    }

    if(*ALUresult == 0) *Zero = 1; else *Zero = 0;
    
}

/* instruction fetch: 10 Points by Poojan */
int instruction_fetch(unsigned PC,unsigned *Mem,unsigned *instruction){

    unsigned i = PC >> 2;

    // Test for word alignment.
    if(PC % 4 != 0) return 1;

    // Get instruction from Memory then return 0;
    *instruction = Mem[i];
    return 0;

}

/* instruction partition: 10 points by All */
void instruction_partition(unsigned instruction, unsigned *op, unsigned *r1,unsigned *r2, unsigned *r3, unsigned *funct, unsigned *offset, unsigned *jsec){

    *op = instruction >> 26;
    *r1 = (instruction >> 21 ) & (0b11111);
    *r2 = (instruction >> 16) & (0b11111);
    *r3 = (instruction >> 11) & (0b11111);
    *funct = instruction & (0b111111);
    *offset = instruction & (0b1111111111111111);
    *jsec = instruction & (0b11111111111111111111111111);

}

/* instruction decode: 15 points by Jacob */ 
int instruction_decode(unsigned op, struct_controls *controls){
  
    //set to deasserted by default
    controls->RegDst = 0x0;
    controls->Jump = 0x0;
    controls->Branch = 0x0;
    controls->MemRead = 0x0;
    controls->MemtoReg = 0x0;
    controls->ALUOp = 0x0;
    controls->MemWrite = 0x0;
    controls->ALUSrc = 0x0;
    controls->RegWrite = 0x0;
    switch (op){
        //update instructions that are enabled
        case 0b000000: //r-type
            controls->RegDst = 0x1;
            controls->ALUOp = 0x7;
            controls->RegWrite = 0x1;
            break;
        case 0b000010: //jump
            controls->RegDst = 0x2;
            controls->Jump = 0x1;
            break;
        case 0b000100: //beq
            controls->RegDst = 0x2;
            controls->Branch = 0x1;
            controls->MemtoReg = 0x2;
            controls->ALUOp = 0x1;
            break;
        case 0b001000: //addi
            controls->ALUSrc = 0x1;
            controls->RegWrite = 0x1;
            break;
        case 0b001010: //slti
            controls->ALUOp = 0x2;
            controls->ALUSrc = 0x1;
            controls->RegWrite = 0x1;
            break;
        case 0b001011: //sltiu
            controls->ALUOp = 0x3;
            controls->ALUSrc = 0x1;
            controls->RegWrite = 0x1;
            break;
        case 0b001111: //lui
            controls->ALUOp = 0x6;
            controls->ALUSrc = 0x1;
            controls->RegWrite = 0x1;
            break;
        case 0b100011: //lw
            controls->MemRead = 0x1;
            controls->MemtoReg = 0x1;
            controls->ALUSrc = 0x1;
            controls->RegWrite = 0x1;
            break;
        case 0b101011: //sw
            controls->RegDst = 0x2;
            controls->MemtoReg = 0x2;
            controls->MemWrite = 0x1;
            controls->ALUSrc = 0x1;
            break;
        default: 
        //Return 1 if a halt condition occurs; otherwise, return 0. 
        return 1;
    }
    return 0;
}

/* Read Register: 5 Points by Juan */
void read_register(unsigned r1,unsigned r2,unsigned *Reg,unsigned *data1,unsigned *data2){
    *data1 = Reg[r1];
    *data2 = Reg[r2];
}

/* Sign Extend: 10 points by Jacob */ 
void sign_extend(unsigned offset,unsigned *extended_value){
    int ones = 0xffff << 16;
    *extended_value = (offset&1<<15?offset|ones:offset);
}

/* ALU operations: 10 Points by Poojan */
int ALU_operations(unsigned data1,unsigned data2,unsigned extended_value,unsigned funct,char ALUOp,char ALUSrc,unsigned *ALUresult,char *Zero){

    if(ALUSrc == 1) data2 = extended_value;

    if(ALUOp == 7){

        switch (funct){

            case 4:
                ALUOp = 6;
                break;
            
            case 32:
                ALUOp = 0;
                break;

            case 34:
                ALUOp = 1;
                break; 

            case 36:
                ALUOp = 4;
                break;

            case 37:
                ALUOp = 5;
                break;
            
            case 39:
                ALUOp = 7;
                break;
            
            case 42:
                ALUOp = 2;
                break;
            
            case 43:
                ALUOp = 3;
                break;
            
            default:
                return 1;

        }
        
        ALU(data1, data2, ALUOp, ALUresult, Zero);

    } else ALU(data1, data2, ALUOp, ALUresult, Zero);

    return 0;

}

/* Read / Write Memory: 10 points by Juan */
int rw_memory(unsigned ALUresult,unsigned data2,char MemWrite,char MemRead,unsigned *memdata,unsigned *Mem){

    if(MemWrite == 1){
        if((ALUresult % 4) == 0) Mem[ALUresult >> 2] = data2;
        //why the >> ? 
         else return 1;
    }

    if(MemRead == 1){
        if((ALUresult % 4) == 0) *memdata = Mem[ALUresult >> 2]; 
        //Why the >> ?
        else return 1;
    }

    return 0;

}

/* Write Register: 10 Points by Juan */
void write_register(unsigned r2,unsigned r3,unsigned memdata,unsigned ALUresult,char RegWrite,char RegDst,char MemtoReg,unsigned *Reg){
    //load word
    if(MemtoReg == 1 && RegDst == 0 && RegWrite == 1){
      //if( (r2 % 4) == 0 ) Reg[r2] = memdata; ~Juan
        Reg[r2] = memdata;
    }
    //R-type
    else if(MemtoReg == 0 && RegDst == 1 &&  RegWrite == 1){
        //if( (r3 % 4) == 0 ) Reg[r3] = ALUresult; ~Juan
        Reg[r3] = ALUresult;
    }
    //addi
    else if(MemtoReg == 0 && RegDst == 0 &&  RegWrite == 1){
        //if( (r3 % 4) == 0 ) Reg[r3] = ALUresult; ~Juan
        Reg[r2] = ALUresult;
    }
  
}

/* PC update: 10 Points by Poojan */
void PC_update(unsigned jsec,unsigned extended_value,char Branch,char Jump,char Zero,unsigned *PC){
    *PC += 4;
    if(Jump == 1) *PC = (jsec << 2) | (*PC & 0xf0000000);
    if(Zero == 1 && Branch == 1) *PC += extended_value << 2;
}
