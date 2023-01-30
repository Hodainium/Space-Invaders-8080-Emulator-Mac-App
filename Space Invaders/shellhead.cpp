//
//  shellhead.cpp
//  Space Invaders
//
//  Created by Holden Scholl on 8/10/19.
//  Copyright © 2019 Holden. All rights reserved.
//

//#include <stdio.h>
#include "shellhead.hpp"

using namespace SHELL;

ShiftPort::ShiftPort()
{
    shiftReg=0;
}

//Machine struct
uint8_t Machine::readShift()
{
    in3 = (shift.shiftReg>>(8-out2))&0xFF;
    return in3;
}

void Machine::writeShift()
{
    shift.shiftReg = (out4<<8)|(shift.shiftReg>>8);
}

Machine::Machine()
{
    in1=0; //attract mode
    in2=0;
    in3=0;
    out2=0;
    out3=0;
    out4=0;
    out5=0;
}

ConditionCodes::ConditionCodes()
{
    z=0;
    s=0;
    p=0;
    ac=0;
    cy=0;
    pad=0;
}

State8080::State8080()
{
    A=0;
    B=0;
    C=0;
    D=0;
    E=0;
    H=0;
    L=0;
    int_enable=0;
    SP=0;
    PC=0;
}

int     last1000index=0;
uint16_t last1000[1000];
uint16_t last1000sp[1000];
uint16_t lastSP;

void PrintLast1000(void)
{
    int i;
    for (i=0; i<100;i++)
    {
        int j;
        printf("%04d ", i*10);
        for (j=0; j<10; j++)
        {
            int n = i*10 + j;
            printf("PC=%04x HL=%04x\n", last1000[n], last1000sp[n]);
            //if (n==last1000index)
                //printf("**");
        }
        printf ("\n");
    }
}


//Functions
int SHELL::parity(int x, int size)
{
    int p=0;
    for(int i=0; i<size; i++){
        if(x&1){
            p++;
        }
        x=x>>1;
    }
    return !(p&1);
}

void SHELL::emulateMachine(Machine * machine)
{
    //check for key presses, also handle shift reg stuff
    
}

void SHELL::generateInt(State8080 * state, int interrupt)
{
    state->memory[state->SP-1]=state->PC>>8; state->memory[state->SP-2]=state->PC; state->SP-=2; //Push PC on stack
    state->PC=interrupt*8;
    state->int_enable = 0;
}

int SHELL::emulate8080(State8080 * state, Machine * machine)
{
    last1000[last1000index] = state->PC;
    last1000sp[last1000index] = ((state->memory[state->SP+1]<<8)|state->memory[state->SP]);
    last1000index++;
    if (last1000index>1000)
        last1000index = 0;
    unsigned char * code = &state->memory[state->PC]; //current byte offset
    uint16_t pair1 = 0;
    uint32_t pair2 = 0; //temp variables for operations
    int cycles = 0;
    switch(*code)
    {
        case 0x00: cycles=4; break;
        case 0x01: state->B=code[2]; state->C=code[1]; state->PC+=2; cycles=10; break; //LXI B, D16
        case 0x02: pair1=(state->B<<8)|state->C; state->memory[pair1]=state->A; cycles=7; break; //STAX B
        case 0x03: state->C++; if(state->C==0) {state->B++;} cycles=5; break; //INX B
        case 0x04: state->B++;
            state->cc.z=(state->B==0);
            state->cc.s=(bool)(state->B&128);
            state->cc.p=parity(state->B); cycles=5;
            break; //INR B
        case 0x05: state->B--; //DCR B
            //flags
            state->cc.z=(state->B==0);
            state->cc.s=(bool)(state->B&128);
            state->cc.p=parity(state->B); cycles=5;
            break;
        case 0x06: state->B=code[1]; state->PC++; cycles=7; break; //MVI B, D8
        case 0x07:
            state->cc.cy=(bool)(state->A&128);
            state->A=((state->cc.cy)|state->A<<1); cycles=4;
            break; //RLC
        case 0x08: cycles=4; break; //NA
        case 0x09: pair1=(state->B<<8)|state->C; pair2=(state->H<<8)|state->L; pair2=pair1+pair2; //DAD B
            state->cc.cy=((pair2&0xFFFF0000)>0); //CY flag
            state->L=pair2; //load into HL
            state->H=pair2>>8; cycles=10;
            break;
        case 0x0a: pair1=(state->B<<8)|state->C; state->A=state->memory[pair1]; cycles=7; break; //LDAX B
        case 0x0b: if(state->C==0)
        {state->B--;
            state->C=255;}
        else {state->C--;} cycles=5;
            break; //DCX B
        case 0x0c: state->C++;
            state->cc.z=(state->C==0); //Flags
            state->cc.s=(bool)(state->C&128);
            state->cc.p=parity(state->C); cycles=5;
            break; //INR C
        case 0x0d: state->C--; //DCR C
            state->cc.z=(state->C==0); //Flags
            state->cc.s=(bool)(state->C&128);
            state->cc.p=parity(state->C); cycles=5;
            break;
        case 0x0e: state->C=code[1]; state->PC++; cycles=7; break; //MVI C, D8
        case 0x0f: //RRC
            state->cc.cy=(bool)(state->A&1);
            state->A=((state->cc.cy<<7)|state->A>>1); cycles=4;
            break;
        case 0x10: cycles=4; break; //NA
        case 0x11: state->D=code[2]; state->E=code[1]; state->PC+=2; cycles=10; break; //LXI D, D16
        case 0x12: pair1=(state->D<<8)|state->E; state->memory[pair1]=state->A; cycles=7; break; //STAX D
        case 0x13: state->E++; if(state->E==0) {state->D++;} cycles=5; break; //INX D
        case 0x14: state->D++;
            state->cc.z=(state->D==0);
            state->cc.s=(bool)(state->D&128);
            state->cc.p=parity(state->D); cycles=5; break; //INR D
        case 0x15: state->D--; //DCR D
            //flags
            state->cc.z=(state->D==0);
            state->cc.s=(bool)(state->D&128);
            state->cc.p=parity(state->D); cycles=5;
            break;
        case 0x16: state->D=code[1]; state->PC++; cycles=7; break; //MVI D
        case 0x17:
            pair1=state->cc.cy; //pair1 holds past value for CY
            state->cc.cy=(bool)(state->A&128);
            state->A=(pair1|state->A<<1); cycles=4;
            break; //RAL
        case 0x18: cycles=4; break; //NA
        case 0x19: pair1=(state->D<<8)|state->E; pair2=(state->H<<8)|state->L; pair2=pair1+pair2;
            state->cc.cy=((pair2&0xFFFF0000)>0);
            state->L=pair2;
            state->H=pair2>>8; cycles=10;
            break; //DAD D DEF FIXED
        case 0x1a: pair1=(state->D<<8)|state->E; state->A=state->memory[pair1]; cycles=7; break; //LDAX D
        case 0x1b: if(state->E==0)
        {state->D--;
            state->E=255;}
        else {state->E--;} cycles=5;
            break; //DCX D
        case 0x1c:  state->E++;
            state->cc.z=(state->E==0);
            state->cc.s=(bool)(state->E&128);
            state->cc.p=parity(state->E); cycles=5;
            break; //INR E
        case 0x1d: state->E--;
            //flags
            state->cc.z=(state->E==0);
            state->cc.s=(bool)(state->E&128);
            state->cc.p=parity(state->E); cycles=5;
            break; //DCR E
        case 0x1e: state->E=code[1]; state->PC++; cycles=7; break; //MVI E
        case 0x1f:
            pair1=state->cc.cy; //pair1 holds past value for CY
            state->cc.cy=(bool)(state->A&1);
            state->A=(pair1<<7|state->A>>1); cycles=4; break; //RAR
        case 0x20: cycles=4; break; //RIM special
        case 0x21: state->H=code[2]; state->L=code[1]; state->PC+=2; cycles=10; break; //LXI H
        case 0x22:
            pair1=(code[2]<<8)|code[1];
            state->memory[pair1]=state->L;
            state->memory[pair1+1]=state->H;
            state->PC+=2; cycles=16;
            break; //SHLD addr
        case 0x23: state->L++; if(state->L==0) {state->H++;} cycles=5; break; //INX H
        case 0x24: state->H++;
            state->cc.z=(state->H==0);
            state->cc.s=(bool)(state->H&128);
            state->cc.p=parity(state->H); cycles=5;
            break; //INR H
        case 0x25: state->H--;
            //flags
            state->cc.z=(state->H==0);
            state->cc.s=(bool)(state->H&128);
            state->cc.p=parity(state->H); cycles=5;
            break; //DCR H
        case 0x26: state->H=code[1]; state->PC++; cycles=7; break; //MVI H D8
        case 0x27: cycles=4;
            if((state->A&0xF)>9||state->cc.ac)//first if need to adjust to check LSB
            {state->A+=0x6;}
            //state->cc.cy=(bool)(pair1&0xFF00);
            if((state->A&0xF0)>0x90||state->cc.cy)//2nd if need to check MSB
            {pair1=state->A+=0x60;}
            state->cc.cy=(bool)(pair1&0xFF00);
            state->cc.z=(state->A==0);
            state->cc.s=(bool)(state->A&128);
            state->cc.p=parity(state->A);
            break; //DAA special
        case 0x28: cycles=4; break; //NA
        case 0x29: pair1=(state->H<<8)|state->L; pair2=(state->H<<8)|state->L; pair2=pair1+pair2; //DAD H
            state->cc.cy=((pair2&0xFFFF0000)>0); //CY flag
            state->L=pair2; //load into HL
            state->H=pair2>>8; cycles=10;
            break;
        case 0x2a: pair1=(code[2]<<8)|code[1];
            state->L=state->memory[pair1];
            state->H=state->memory[pair1+1];
            state->PC+=2; cycles=16;
            break; //LHLD addr
        case 0x2b: if(state->L==0)
        {state->H--;
            state->L=255;}
        else {state->L--;} cycles=5;
            break; //DCX H
        case 0x2c: state->L++;
            state->cc.z=(state->L==0);
            state->cc.s=(bool)(state->L&128);
            state->cc.p=parity(state->L); cycles=5;
            break; //INR L CYCLES ARE NOW IN BEGINNING OF CASE----------------------------------------------
        case 0x2d: cycles=5; state->L--;
            //flags
            state->cc.z=(state->L==0);
            state->cc.s=(bool)(state->L&128);
            state->cc.p=parity(state->L);
            break; //DCR L
        case 0x2e: cycles=7; state->L=code[1]; state->PC++; break; //MVI L
        case 0x2f: cycles=4; state->A=~state->A; break; //CMA needs testing
        case 0x30: cycles=4; break; //SIM special
        case 0x31: cycles=10; state->SP=(code[2]<<8)|code[1]; state->PC+=2; break; //LXI SP
        case 0x32: cycles=13; pair1=(code[2]<<8)|code[1]; state->memory[pair1]=state->A; state->PC+=2; break; //STA addr
        case 0x33: cycles=5; state->SP++; break; //INX SP
        case 0x34: cycles=10; pair1=(state->H<<8)|state->L;
            state->memory[pair1]=state->memory[pair1]+1;
            pair1=state->memory[pair1];
            state->cc.z=(pair1==0);
            state->cc.s=(bool)(pair1&128);
            state->cc.p=parity(pair1);
            break; //INR M needs testing
        case 0x35: cycles=10; pair1=(state->H<<8)|state->L;
            state->memory[pair1]=state->memory[pair1]-1;
            pair1=state->memory[pair1];
            state->cc.z=(pair1==0);
            state->cc.s=(bool)(pair1&128);
            state->cc.p=parity(pair1);
            break; //DCR M
        case 0x36: cycles=10; pair1=(state->H<<8)|state->L; state->memory[pair1]=code[1]; state->PC++; break; //MVI M, D8
        case 0x37: cycles=4; state->cc.cy=true; break;
        case 0x38: cycles=4; break; //NA
        case 0x39: cycles=10; pair2=(state->H<<8)|state->L; pair2=state->SP+pair2; //DAD SP
            state->cc.cy=((pair2&0xFFFF0000)>0); //CY flag
            state->L=pair2; //load into HL
            state->H=pair2>>8;
            break;
        case 0x3a: cycles=13; pair1=(code[2]<<8)|code[1]; state->A=state->memory[pair1]; state->PC+=2; break; //LDA addr
        case 0x3b: cycles=5; state->SP--; break; //DCX SP
        case 0x3c: cycles=5; state->A++;
            state->cc.z=(state->A==0);
            state->cc.s=(bool)(state->A&128);
            state->cc.p=parity(state->A);
            break; //INR A
        case 0x3d: cycles=5; state->A--;
            //flags
            state->cc.z=(state->A==0);
            state->cc.s=(bool)(state->A&128);
            state->cc.p=parity(state->A);
            break; //DCR A
        case 0x3e: cycles=7; state->A=code[1]; state->PC++; break; //MVI A
        case 0x3f: cycles=4; state->cc.cy=~state->cc.cy; break; //CMC
        case 0x40: cycles=5; state->B=state->B; break; //MOV B, B
        case 0x41: cycles=5; state->B=state->C; break; //MOV B, C
        case 0x42: cycles=5; state->B=state->D; break; //MOV B, D
        case 0x43: cycles=5; state->B=state->E; break; //MOV B, E
        case 0x44: cycles=5; state->B=state->H; break; //MOV B, H
        case 0x45: cycles=5; state->B=state->L; break; //MOV B, L
        case 0x46: cycles=7; pair1=(state->H<<8)|state->L; state->B=state->memory[pair1]; break; //MOV B, M
        case 0x47: cycles=5; state->B=state->A; break; //MOV B, A
        case 0x48: cycles=5; state->C=state->B; break; //MOV C, B
        case 0x49: cycles=5; state->C=state->C; break; //MOV C, C
        case 0x4a: cycles=5; state->C=state->D; break; //MOV C, D
        case 0x4b: cycles=5; state->C=state->E; break; //MOV C, E
        case 0x4c: cycles=5; state->C=state->H; break; //MOV C, H
        case 0x4d: cycles=5; state->C=state->L; break; //MOV C, L
        case 0x4e: cycles=7; pair1=(state->H<<8)|state->L; state->C=state->memory[pair1]; break; //MOV C, M
        case 0x4f: cycles=5; state->C=state->A; break; //MOV C, A
        case 0x50: cycles=5; state->D=state->B; break; //MOV D, B
        case 0x51: cycles=5; state->D=state->C; break; //MOV D, C
        case 0x52: cycles=5; state->D=state->D; break; //MOV D, D
        case 0x53: cycles=5; state->D=state->E; break; //MOV D, E
        case 0x54: cycles=5; state->D=state->H; break; //MOV D, H
        case 0x55: cycles=5; state->D=state->L; break; //MOV D, L
        case 0x56: cycles=7; pair1=(state->H<<8)|state->L; state->D=state->memory[pair1]; break; //MOV D, M
        case 0x57: cycles=5; state->D=state->A; break; //MOV D, A
        case 0x58: cycles=5; state->E=state->B; break; //MOV E, B
        case 0x59: cycles=5; state->E=state->C; break; //MOV E, C
        case 0x5a: cycles=5; state->E=state->D; break; //MOV E, D
        case 0x5b: cycles=5; state->E=state->E; break; //MOV E, E
        case 0x5c: cycles=5; state->E=state->H; break; //MOV E, H
        case 0x5d: cycles=5; state->E=state->L; break; //MOV E, L
        case 0x5e: cycles=7; pair1=(state->H<<8)|state->L; state->E=state->memory[pair1]; break; //MOV E, M
        case 0x5f: cycles=5; state->E=state->A; break; //MOV E, A
        case 0x60: cycles=5; state->H=state->B; break; //MOV H, B
        case 0x61: cycles=5; state->H=state->C; break; //MOV H, C
        case 0x62: cycles=5; state->H=state->D; break; //MOV H, D
        case 0x63: cycles=5; state->H=state->E; break; //MOV H, E
        case 0x64: cycles=5; state->H=state->H; break; //MOV H, H
        case 0x65: cycles=5; state->H=state->L; break; //MOV H, L
        case 0x66: cycles=7; pair1=(state->H<<8)|state->L; state->H=state->memory[pair1]; break; //MOV H, M
        case 0x67: cycles=5; state->H=state->A; break; //MOV H, A
        case 0x68: cycles=5; state->L=state->B; break; //MOV L, B
        case 0x69: cycles=5; state->L=state->C; break; //MOV L, C
        case 0x6a: cycles=5; state->L=state->D; break; //MOV L, D
        case 0x6b: cycles=5; state->L=state->E; break; //MOV L, E
        case 0x6c: cycles=5; state->L=state->H; break; //MOV L, H
        case 0x6d: cycles=5; state->L=state->L; break; //MOV L, L
        case 0x6e: cycles=7; pair1=(state->H<<8)|state->L; state->L=state->memory[pair1]; break; //MOV L, M
        case 0x6f: cycles=5; state->L=state->A; break; //MOV L, A
        case 0x70: cycles=7; pair1=(state->H<<8)|state->L; state->memory[pair1]=state->B; break; //MOV M, B
        case 0x71: cycles=7; pair1=(state->H<<8)|state->L; state->memory[pair1]=state->C; break; //MOV M, C
        case 0x72: cycles=7; pair1=(state->H<<8)|state->L; state->memory[pair1]=state->D; break; //MOV M, D
        case 0x73: cycles=7; pair1=(state->H<<8)|state->L; state->memory[pair1]=state->E; break; //MOV M, E
        case 0x74: cycles=7; pair1=(state->H<<8)|state->L; state->memory[pair1]=state->H; break; //MOV M, H
        case 0x75: cycles=7; pair1=(state->H<<8)|state->L; state->memory[pair1]=state->L; break; //MOV M, L
        case 0x76: cycles=7; break; //HLT special
        case 0x77: cycles=7; pair1=(state->H<<8)|state->L; state->memory[pair1]=state->A; break; //MOV M, A
        case 0x78: cycles=5; state->A=state->B; break; //MOV A, B
        case 0x79: cycles=5; state->A=state->C; break; //MOV A, C
        case 0x7a: cycles=5; state->A=state->D; break; //MOV A, D
        case 0x7b: cycles=5; state->A=state->E; break; //MOV A, E
        case 0x7c: cycles=5; state->A=state->H; break; //MOV A, H
        case 0x7d: cycles=5; state->A=state->L; break; //MOV A, L
        case 0x7e: cycles=7; pair1=(state->H<<8)|state->L; state->A=state->memory[pair1]; break; //MOV A, M
        case 0x7f: cycles=5; state->A=state->A; break; //MOV A, A
        case 0x80: cycles=4; state->A=pair1=state->A+state->B;
            state->cc.cy=((pair1&0xFF00)>0); //CY flag
            state->cc.z=(state->A==0);
            state->cc.s=(bool)(state->A&128);
            state->cc.p=parity(state->A);
            break; //ADD B
        case 0x81: cycles=4; state->A=pair1=state->A+state->C;
            state->cc.cy=((pair1&0xFF00)>0); //CY flag
            state->cc.z=(state->A==0);
            state->cc.s=(bool)(state->A&128);
            state->cc.p=parity(state->A);
            break; //ADD C
        case 0x82: cycles=4; state->A=pair1=state->A+state->D;
            state->cc.cy=((pair1&0xFF00)>0); //CY flag
            state->cc.z=(state->A==0);
            state->cc.s=(bool)(state->A&128);
            state->cc.p=parity(state->A);
            break; //ADD D
        case 0x83: cycles=4; state->A=pair1=state->A+state->E;
            state->cc.cy=((pair1&0xFF00)>0); //CY flag
            state->cc.z=(state->A==0);
            state->cc.s=(bool)(state->A&128);
            state->cc.p=parity(state->A);
            break; //ADD E
        case 0x84: cycles=4; state->A=pair1=state->A+state->H;
            state->cc.cy=((pair1&0xFF00)>0); //CY flag
            state->cc.z=(state->A==0);
            state->cc.s=(bool)(state->A&128);
            state->cc.p=parity(state->A);
            break; //ADD H
        case 0x85: cycles=4; state->A=pair1=state->A+state->L;
            state->cc.cy=((pair1&0xFF00)>0); //CY flag
            state->cc.z=(state->A==0);
            state->cc.s=(bool)(state->A&128);
            state->cc.p=parity(state->A);
            break; //ADD L
        case 0x86: cycles=7; pair1=(state->H<<8)|state->L;
            state->A=pair1=state->A+state->memory[pair1];
            state->cc.cy=((pair1&0xFF00)>0); //CY flag
            state->cc.z=(state->A==0);
            state->cc.s=(bool)(state->A&128);
            state->cc.p=parity(state->A);
            break; //ADD M
        case 0x87: cycles=4; state->A=pair1=state->A+state->A;
            state->cc.cy=((pair1&0xFF00)>0); //CY flag
            state->cc.z=(state->A==0);
            state->cc.s=(bool)(state->A&128);
            state->cc.p=parity(state->A);
            break; //ADD A
        case 0x88: cycles=4; state->A=pair1=state->A+state->B+state->cc.cy;
            state->cc.cy=((pair1&0xFF00)>0); //CY flag
            state->cc.z=(state->A==0);
            state->cc.s=(bool)(state->A&128);
            state->cc.p=parity(state->A);
            break; //ADC B
        case 0x89: cycles=4; state->A=pair1=state->A+state->C+state->cc.cy;
            state->cc.cy=((pair1&0xFF00)>0); //CY flag
            state->cc.z=(state->A==0);
            state->cc.s=(bool)(state->A&128);
            state->cc.p=parity(state->A);
            break; //ADC C
        case 0x8a: cycles=4; state->A=pair1=state->A+state->D+state->cc.cy;
            state->cc.cy=((pair1&0xFF00)>0); //CY flag
            state->cc.z=(state->A==0);
            state->cc.s=(bool)(state->A&128);
            state->cc.p=parity(state->A);
            break; //ADC D
        case 0x8b: cycles=4; state->A=pair1=state->A+state->E+state->cc.cy;
            state->cc.cy=((pair1&0xFF00)>0); //CY flag
            state->cc.z=(state->A==0);
            state->cc.s=(bool)(state->A&128);
            state->cc.p=parity(state->A);
            break; //ADC E
        case 0x8c: cycles=4; state->A=pair1=state->A+state->H+state->cc.cy;
            state->cc.cy=((pair1&0xFF00)>0); //CY flag
            state->cc.z=(state->A==0);
            state->cc.s=(bool)(state->A&128);
            state->cc.p=parity(state->A);
            break; //ADC H
        case 0x8d: cycles=4; state->A=pair1=state->A+state->L+state->cc.cy;
            state->cc.cy=((pair1&0xFF00)>0); //CY flag
            state->cc.z=(state->A==0);
            state->cc.s=(bool)(state->A&128);
            state->cc.p=parity(state->A);
            break; //ADC L
        case 0x8e: cycles=7; pair1=(state->H<<8)|state->L;
            state->A=pair1=state->A+state->memory[pair1]+state->cc.cy;
            state->cc.cy=((pair1&0xFF00)>0); //CY flag
            state->cc.z=(state->A==0);
            state->cc.s=(bool)(state->A&128);
            state->cc.p=parity(state->A);
            break; //ADC M
        case 0x8f: cycles=4; state->A=pair1=state->A+state->A+state->cc.cy;
            state->cc.cy=((pair1&0xFF00)>0); //CY flag
            state->cc.z=(state->A==0);
            state->cc.s=(bool)(state->A&128);
            state->cc.p=parity(state->A);
            break; //ADC A
        case 0x90: cycles=4; state->A=pair1=state->A-state->B;
            state->cc.cy=((pair1&0xFF00)>0); //CY flag
            state->cc.z=(state->A==0);
            state->cc.s=(bool)(state->A&128);
            state->cc.p=parity(state->A);
            break; //SUB B
        case 0x91: cycles=4; state->A=pair1=state->A-state->C;
            state->cc.cy=((pair1&0xFF00)>0); //CY flag
            state->cc.z=(state->A==0);
            state->cc.s=(bool)(state->A&128);
            state->cc.p=parity(state->A);
            break; //SUB C
        case 0x92: cycles=4; state->A=pair1=state->A-state->D;
            state->cc.cy=((pair1&0xFF00)>0); //CY flag
            state->cc.z=(state->A==0);
            state->cc.s=(bool)(state->A&128);
            state->cc.p=parity(state->A);
            break; //SUB D
        case 0x93: cycles=4; state->A=pair1=state->A-state->E;
            state->cc.cy=((pair1&0xFF00)>0); //CY flag
            state->cc.z=(state->A==0);
            state->cc.s=(bool)(state->A&128);
            state->cc.p=parity(state->A);
            break; //SUB E
        case 0x94: cycles=4; state->A=pair1=state->A-state->H;
            state->cc.cy=((pair1&0xFF00)>0); //CY flag
            state->cc.z=(state->A==0);
            state->cc.s=(bool)(state->A&128);
            state->cc.p=parity(state->A);
            break; //SUB H
        case 0x95: cycles=4; state->A=pair1=state->A-state->L;
            state->cc.cy=((pair1&0xFF00)>0); //CY flag
            state->cc.z=(state->A==0);
            state->cc.s=(bool)(state->A&128);
            state->cc.p=parity(state->A);
            break; //SUB L
        case 0x96: cycles=7; pair1=(state->H<<8)|state->L;
            state->A=pair1=state->A-state->memory[pair1];
            state->cc.cy=((pair1&0xFF00)>0); //CY flag
            state->cc.z=(state->A==0);
            state->cc.s=(bool)(state->A&128);
            state->cc.p=parity(state->A);
            break; //SUB M
        case 0x97: cycles=4; state->A=pair1=state->A-state->A;
            state->cc.cy=((pair1&0xFF00)>0); //CY flag
            state->cc.z=(state->A==0);
            state->cc.s=(bool)(state->A&128);
            state->cc.p=parity(state->A);
            break; //SUB A
        case 0x98: cycles=4; state->A=pair1=state->A-state->B-state->cc.cy;
            state->cc.cy=((pair1&0xFF00)>0); //CY flag
            state->cc.z=(state->A==0);
            state->cc.s=(bool)(state->A&128);
            state->cc.p=parity(state->A);
            break; //SBB B
        case 0x99: cycles=4; state->A=pair1=state->A-state->C-state->cc.cy;
            state->cc.cy=((pair1&0xFF00)>0); //CY flag
            state->cc.z=(state->A==0);
            state->cc.s=(bool)(state->A&128);
            state->cc.p=parity(state->A);
            break; //SBB C
        case 0x9a: cycles=4; state->A=pair1=state->A-state->D-state->cc.cy;
            state->cc.cy=((pair1&0xFF00)>0); //CY flag
            state->cc.z=(state->A==0);
            state->cc.s=(bool)(state->A&128);
            state->cc.p=parity(state->A);
            break; //SBB D
        case 0x9b: cycles=4; state->A=pair1=state->A-state->E-state->cc.cy;
            state->cc.cy=((pair1&0xFF00)>0); //CY flag
            state->cc.z=(state->A==0);
            state->cc.s=(bool)(state->A&128);
            state->cc.p=parity(state->A);
            break; //SBB E
        case 0x9c: cycles=4; state->A=pair1=state->A-state->H-state->cc.cy;
            state->cc.cy=((pair1&0xFF00)>0); //CY flag
            state->cc.z=(state->A==0);
            state->cc.s=(bool)(state->A&128);
            state->cc.p=parity(state->A);
            break; //SBB H
        case 0x9d: cycles=4; state->A=pair1=state->A-state->L-state->cc.cy;
            state->cc.cy=((pair1&0xFF00)>0); //CY flag
            state->cc.z=(state->A==0);
            state->cc.s=(bool)(state->A&128);
            state->cc.p=parity(state->A);
            break; //SBB L
        case 0x9e: cycles=7; pair1=(state->H<<8)|state->L;
            state->A=pair1=state->A-state->memory[pair1]-state->cc.cy;
            state->cc.cy=((pair1&0xFF00)>0); //CY flag
            state->cc.z=(state->A==0);
            state->cc.s=(bool)(state->A&128);
            state->cc.p=parity(state->A);
            break; //SBB M
        case 0x9f: cycles=4; state->A=pair1=state->A-state->A-state->cc.cy;
            state->cc.cy=((pair1&0xFF00)>0); //CY flag
            state->cc.z=(state->A==0);
            state->cc.s=(bool)(state->A&128);
            state->cc.p=parity(state->A);
            break; //SBB A
        case 0xa0: cycles=4; state->A=(state->A&state->B);
            state->cc.cy=0; //CY flag
            state->cc.z=(state->A==0);
            state->cc.s=(bool)(state->A&128);
            state->cc.p=parity(state->A);
            break; //ANA B
        case 0xa1: cycles=4; state->A=(state->A&state->C);
            state->cc.cy=0; //CY flag
            state->cc.z=(state->A==0);
            state->cc.s=(bool)(state->A&128);
            state->cc.p=parity(state->A);
            break; //ANA C
        case 0xa2: cycles=4; state->A=(state->A&state->D);
            state->cc.cy=0; //CY flag
            state->cc.z=(state->A==0);
            state->cc.s=(bool)(state->A&128);
            state->cc.p=parity(state->A);
            break; //ANA D
        case 0xa3: cycles=4; state->A=(state->A&state->E);
            state->cc.cy=0; //CY flag
            state->cc.z=(state->A==0);
            state->cc.s=(bool)(state->A&128);
            state->cc.p=parity(state->A);
            break; //ANA E
        case 0xa4: cycles=4; state->A=(state->A&state->H);
            state->cc.cy=0; //CY flag
            state->cc.z=(state->A==0);
            state->cc.s=(bool)(state->A&128);
            state->cc.p=parity(state->A);
            break; //ANA H
        case 0xa5: cycles=4; state->A=(state->A&state->L);
            state->cc.cy=0; //CY flag
            state->cc.z=(state->A==0);
            state->cc.s=(bool)(state->A&128);
            state->cc.p=parity(state->A);
            break; //ANA L
        case 0xa6: cycles=7; pair1=(state->H<<8)|state->L;
            state->A=(state->A&state->memory[pair1]);
            state->cc.cy=0; //CY flag
            state->cc.z=(state->A==0);
            state->cc.s=(bool)(state->A&128);
            state->cc.p=parity(state->A);
            break; //ANA M
        case 0xa7: cycles=4; state->A=(state->A&state->A);
            state->cc.z=(state->A==0);
            state->cc.s=(bool)(state->A&128);
            state->cc.p=parity(state->A);
            state->cc.cy=0; //CY flag
            break; //ANA A
        case 0xa8: cycles=4; state->A=(state->A^state->B);
            state->cc.z=(state->A==0);
            state->cc.s=(bool)(state->A&128);
            state->cc.p=parity(state->A);
            state->cc.cy=0; //CY flag
            break; //XRA B
        case 0xa9: cycles=4; state->A=(state->A^state->C);
            state->cc.z=(state->A==0);
            state->cc.s=(bool)(state->A&128);
            state->cc.p=parity(state->A);
            state->cc.cy=0; //CY flag
            break; //XRA C
        case 0xaa: cycles=4; state->A=(state->A^state->D);
            state->cc.z=(state->A==0);
            state->cc.s=(bool)(state->A&128);
            state->cc.p=parity(state->A);
            state->cc.cy=0; //CY flag
            break; //XRA D
        case 0xab: cycles=4; state->A=(state->A^state->E);
            state->cc.z=(state->A==0);
            state->cc.s=(bool)(state->A&128);
            state->cc.p=parity(state->A);
            state->cc.cy=0; //CY flag
            break; //XRA E
        case 0xac: cycles=4; state->A=(state->A^state->H);
            state->cc.z=(state->A==0);
            state->cc.s=(bool)(state->A&128);
            state->cc.p=parity(state->A);
            state->cc.cy=0; //CY flag
            break; //XRA H
        case 0xad: cycles=4; state->A=(state->A^state->L);
            state->cc.z=(state->A==0);
            state->cc.s=(bool)(state->A&128);
            state->cc.p=parity(state->A);
            state->cc.cy=0; //CY flag
            break; //XRA L
        case 0xae: cycles=7; pair1=(state->H<<8)|state->L;
            state->A=(state->A^state->memory[pair1]);
            state->cc.z=(state->A==0);
            state->cc.s=(bool)(state->A&128);
            state->cc.p=parity(state->A);
            state->cc.cy=0; //CY flag
            break; //XRA M
        case 0xaf: cycles=4; state->A=(state->A^state->A);
            state->cc.z=(state->A==0);
            state->cc.s=(bool)(state->A&128);
            state->cc.p=parity(state->A);
            state->cc.cy=0; //CY flag
            break; //XRA A
        case 0xb0: cycles=4; state->A=(state->A|state->B);
            state->cc.z=(state->A==0);
            state->cc.s=(bool)(state->A&128);
            state->cc.p=parity(state->A);
            state->cc.cy=0; //CY flag
            break; //ORA B
        case 0xb1: cycles=4; state->A=(state->A|state->C);
            state->cc.z=(state->A==0);
            state->cc.s=(bool)(state->A&128);
            state->cc.p=parity(state->A);
            state->cc.cy=0; //CY flag
            break; //ORA C
        case 0xb2: cycles=4; state->A=(state->A|state->D);
            state->cc.z=(state->A==0);
            state->cc.s=(bool)(state->A&128);
            state->cc.p=parity(state->A);
            state->cc.cy=0; //CY flag
            break; //ORA D
        case 0xb3: cycles=4; state->A=(state->A|state->E);
            state->cc.z=(state->A==0);
            state->cc.s=(bool)(state->A&128);
            state->cc.p=parity(state->A);
            state->cc.cy=0; //CY flag
            break; //ORA E
        case 0xb4: cycles=4; state->A=(state->A|state->H);
            state->cc.z=(state->A==0);
            state->cc.s=(bool)(state->A&128);
            state->cc.p=parity(state->A);
            state->cc.cy=0; //CY flag
            break; //ORA H
        case 0xb5: cycles=4; state->A=(state->A|state->L);
            state->cc.z=(state->A==0);
            state->cc.s=(bool)(state->A&128);
            state->cc.p=parity(state->A);
            state->cc.cy=0; //CY flag
            break; //ORA L
        case 0xb6: cycles=7; pair1=(state->H<<8)|state->L;
            state->A=(state->A|state->memory[pair1]);
            state->cc.z=(state->A==0);
            state->cc.s=(bool)(state->A&128);
            state->cc.p=parity(state->A);
            state->cc.cy=0; //CY flag
            break; //ORA M
        case 0xb7: cycles=4; state->A=(state->A|state->A);
            state->cc.z=(state->A==0);
            state->cc.s=(bool)(state->A&128);
            state->cc.p=parity(state->A);
            state->cc.cy=0; //CY flag
            break; //ORA A
        case 0xb8: cycles=4; pair1=state->A-state->B;
            state->cc.cy=(state->A<state->B); //CY flag
            state->cc.z=(state->A==state->B);
            state->cc.s=(bool)(pair1&128);
            state->cc.p=parity(pair1);
            state->PC++;
            break; //CMP B
        case 0xb9: cycles=4; pair1=state->A-state->C;
            state->cc.cy=(state->A<state->C); //CY flag
            state->cc.z=(state->A==state->C);
            state->cc.s=(bool)(pair1&128);
            state->cc.p=parity(pair1);
            break; //CMP C
        case 0xba: cycles=4; pair1=state->A-state->D;
            state->cc.cy=(state->A<state->D); //CY flag
            state->cc.z=(state->A==state->D);
            state->cc.s=(bool)(pair1&128);
            state->cc.p=parity(pair1);
            break; //CMP D
        case 0xbb: cycles=4; pair1=state->A-state->E;
            state->cc.cy=(state->A<state->E); //CY flag
            state->cc.z=(state->A==state->E);
            state->cc.s=(bool)(pair1&128);
            state->cc.p=parity(pair1);
            break; //CMP E
        case 0xbc: cycles=4; pair1=state->A-state->H;
            state->cc.cy=(state->A<state->H); //CY flag
            state->cc.z=(state->A==state->H);
            state->cc.s=(bool)(pair1&128);
            state->cc.p=parity(pair1);
            break; //CMP H
        case 0xbd: cycles=4; pair1=state->A-state->L;
            state->cc.cy=(state->A<state->L); //CY flag
            state->cc.z=(state->A==state->L);
            state->cc.s=(bool)(pair1&128);
            state->cc.p=parity(pair1);
            break; //CMP L
        case 0xbe: cycles=7; pair2=(state->H<<8)|state->L;
            pair1=state->A-state->memory[pair2];
            state->cc.cy=(state->A<state->memory[pair2]); //CY flag
            state->cc.z=(state->A==state->memory[pair2]);
            state->cc.s=(bool)(pair1&128);
            state->cc.p=parity(pair1);
            break; //CMP M
        case 0xbf: cycles=4; pair1=state->A-state->A;
            state->cc.cy=(state->A<state->A); //CY flag
            state->cc.z=(state->A==state->A);
            state->cc.s=(bool)(pair1&128);
            state->cc.p=parity(pair1);
            break; //CMP A
        case 0xc0: cycles=11; if(!(state->cc.z))
        {state->PC=((state->memory[state->SP+1]<<8)|state->memory[state->SP]); state->SP+=2; state->PC--;} //return
            break; //RNZ
        case 0xc1: cycles=10; state->C=state->memory[state->SP]; state->B=state->memory[state->SP+1]; state->SP+=2; break; //POP B
        case 0xc2: cycles=10; if(!(state->cc.z))
        {state->PC=(code[2]<<8)|code[1];
            state->PC--;}
        else
        {
            state->PC+=2;
        }
            break; //JNZ addr NOT TESTED, EDITED
        case 0xc3: cycles=10; state->PC=(code[2]<<8)|code[1]; state->PC--; break; //JMP NOT TESTED
        case 0xc4: cycles=17; if(!(state->cc.z))
        {state->PC+=3; state->memory[state->SP-1]=state->PC>>8; state->memory[state->SP-2]=state->PC; state->SP-=2;
            state->PC=(code[2]<<8)|code[1]; state->PC--; break;}//call
        else
        {
            state->PC+=2;
        }
            break; //CNZ
        case 0xc5: cycles=11; state->memory[state->SP-1]=state->B; state->memory[state->SP-2]=state->C; state->SP-=2; break; //PUSH B
        case 0xc6: cycles=7; pair1=state->A+code[1]; state->PC++;
            state->cc.cy=((pair1&0xFF00)>0); //CY flag
            state->A=pair1;
            state->cc.z=(state->A==0);
            state->cc.s=(bool)(state->A&128);
            state->cc.p=parity(state->A);
            break; //ADI, D8
        case 0xc7: cycles=11; state->PC++; state->memory[state->SP-1]=state->PC>>8; state->memory[state->SP-2]=state->PC; state->SP-=2;
            state->PC=0; return cycles; //RST 0 ($0)
        case 0xc8: cycles=11; if(state->cc.z)
        {state->PC=((state->memory[state->SP+1]<<8)|state->memory[state->SP]); state->SP+=2; state->PC--;} //return
            break; //RZ
        case 0xc9: cycles=10; state->PC=((state->memory[state->SP+1]<<8)|state->memory[state->SP]); state->SP+=2; state->PC--; break; //RETURN
        case 0xca: cycles=10; if(state->cc.z)
        {state->PC=(code[2]<<8)|code[1]; state->PC--;}
        else
        {
            state->PC+=2;
        }
            break; //JZ addr
        case 0xcb: cycles=10; break; //NA
        case 0xcc: cycles=10; if(state->cc.z)
        {state->PC+=3; state->memory[state->SP-1]=state->PC>>8; state->memory[state->SP-2]=state->PC; state->SP-=2;
            state->PC=(code[2]<<8)|code[1]; state->PC--; break;} //call
        else
        {
            state->PC+=2;
        }
            break; //CZ addr
        case 0xcd: cycles=17; state->PC+=3; state->memory[state->SP-1]=state->PC>>8; state->memory[state->SP-2]=state->PC; state->SP-=2;
            state->PC=(code[2]<<8)|code[1]; state->PC--; break; //CALL addr
        case 0xce: cycles=7; pair1=state->A+code[1]+state->cc.cy; state->PC++;
            state->cc.cy=((pair1&0xFF00)>0); //CY flag
            state->A=pair1;
            state->cc.z=(state->A==0);
            state->cc.s=(bool)(state->A&128);
            state->cc.p=parity(state->A);
            break; //ACI, D8
        case 0xcf: cycles=11; state->PC++; state->memory[state->SP-1]=state->PC>>8; state->memory[state->SP-2]=state->PC; state->SP-=2;
            state->PC=8; return cycles; //RST 1 ($8)
        case 0xd0: cycles=10; if(!(state->cc.cy))
        {state->PC=((state->memory[state->SP+1]<<8)|state->memory[state->SP]); state->SP+=2; state->PC--;} //return
            break; //RNC
        case 0xd1: cycles=10; state->E=state->memory[state->SP]; state->D=state->memory[state->SP+1]; state->SP+=2; break; //POP D
        case 0xd2: cycles=10; if(!(state->cc.cy))
        {state->PC=(code[2]<<8)|code[1];
            state->PC--;}
        else
        {
            state->PC+=2;
        }
            break; //JNC
        case 0xd3: cycles=17;
            switch(code[1])
        {
            case 2: machine->out2=state->A; break; //out2
            case 3: machine->out3=state->A; break; //out3
            case 4: machine->out4=state->A; machine->writeShift(); break; //out4 and write to shiftReg
            case 5: machine->out5=state->A; break; //out5
        }
            state->PC++;
            break; //OUT D8 SPECIAL INPRO
        case 0xd4: cycles=11; if(!(state->cc.cy))
        {state->PC+=3; state->memory[state->SP-1]=state->PC>>8; state->memory[state->SP-2]=state->PC; state->SP-=2;
            state->PC=(code[2]<<8)|code[1]; state->PC--; break;} //call
        else
        {
            state->PC+=2;
        }
            break; //CNC addr
        case 0xd5: cycles=7; state->memory[state->SP-1]=state->D; state->memory[state->SP-2]=state->E; state->SP-=2; break; //PUSH D
        case 0xd6: cycles=11; pair1=state->A-code[1]; state->PC++;
            state->cc.cy=((pair1&0xFF00)>0); //CY flag
            state->A=pair1;
            state->cc.z=(state->A==0);
            state->cc.s=(bool)(state->A&128);
            state->cc.p=parity(state->A);
            break; //SUI, D8
        case 0xd7: cycles=11; state->PC++; state->memory[state->SP-1]=state->PC>>8; state->memory[state->SP-2]=state->PC; state->SP-=2;
            state->PC=16; return cycles; //RST 2 ($10)
        case 0xd8: cycles=10; if(state->cc.cy)
        {state->PC=((state->memory[state->SP+1]<<8)|state->memory[state->SP]); state->SP+=2; state->PC--;} //return
            break; //RC
        case 0xd9: cycles=10; break; //NA
        case 0xda: cycles=10; if(state->cc.cy)
        {state->PC=(code[2]<<8)|code[1];
            state->PC--;}
        else
        {
            state->PC+=2;
        }
            break; //JC
        case 0xdb: cycles=10; //machineIN(state, code[1]); state->PC++;
            switch(code[1])
        {
            case 1: state->A=machine->in1; break; //in1
            case 2: state->A=machine->in2; break; //in2
            case 3: state->A=machine->readShift(); break; //in3
        }
            state->PC++;
            break; //IN D8 SPECIAL INPRO
        case 0xdc: cycles=17; if(state->cc.cy)
        {state->PC+=3; state->memory[state->SP-1]=state->PC>>8; state->memory[state->SP-2]=state->PC; state->SP-=2;
            state->PC=(code[2]<<8)|code[1]; state->PC--; break;} //call
        else
        {
            state->PC+=2;
        }
            break; //CC addr
        case 0xdd: cycles=7; break; //NA
        case 0xde: cycles=11; pair1=state->A-code[1]-state->cc.cy; state->PC++;
            state->cc.cy=((pair1&0xFF00)>0); //CY flag
            state->A=pair1;
            state->cc.z=(state->A==0);
            state->cc.s=(bool)(state->A&128);
            state->cc.p=parity(state->A);
            break; //SBI, D8
        case 0xdf: cycles=11; state->PC++; state->memory[state->SP-1]=state->PC>>8; state->memory[state->SP-2]=state->PC; state->SP-=2;
            state->PC=24; return cycles; //RST 3 ($18)
        case 0xe0: if(!(state->cc.p))
        {state->PC=((state->memory[state->SP+1]<<8)|state->memory[state->SP]); state->SP+=2; state->PC--;} //return
            break; //RPO
        case 0xe1: cycles=10; state->L=state->memory[state->SP]; state->H=state->memory[state->SP+1]; state->SP+=2; break; //POP H
        case 0xe2: cycles=10; if(!(state->cc.p))
        {state->PC=(code[2]<<8)|code[1]; state->PC--;}
        else
        {
            state->PC+=2;
        }
            break; //JPO
        case 0xe3: cycles=18; pair1=state->memory[state->SP]; pair2=state->memory[state->SP+1];
            state->memory[state->SP]=state->L; state->memory[state->SP+1]=state->H;
            state->L=pair1; state->H=pair2; break; //XTHL
        case 0xe4: cycles=17; if(!(state->cc.p))
        {state->PC+=3; state->memory[state->SP-1]=state->PC>>8; state->memory[state->SP-2]=state->PC; state->SP-=2;
            state->PC=(code[2]<<8)|code[1]; state->PC--; break;} //call
        else
        {
            state->PC+=2;
        }
            break; //CPO addr
        case 0xe5: cycles=11; state->memory[state->SP-1]=state->H; state->memory[state->SP-2]=state->L; state->SP-=2; break; //PUSH H
        case 0xe6: cycles=7; state->A=state->A&code[1]; state->PC++;
            state->cc.z=(state->A==0);
            state->cc.s=(bool)(state->A&128);
            state->cc.p=parity(state->A);
            break; //ANI D8
        case 0xe7: cycles=11; state->PC++; state->memory[state->SP-1]=state->PC>>8; state->memory[state->SP-2]=state->PC; state->SP-=2;
            state->PC=32; return cycles; //RST 4 ($20)
        case 0xe8: cycles=11; if(state->cc.p)
        {state->PC=((state->memory[state->SP+1]<<8)|state->memory[state->SP]); state->SP+=2; state->PC--;} //return
            break; //RPE
        case 0xe9: cycles=5; state->PC=(state->H<<8)|state->L; state->PC--; break; //PCHL
        case 0xea: cycles=10; if(state->cc.p)
        {state->PC=(code[2]<<8)|code[1]; state->PC--;}
        else
        {
            state->PC+=2;
        }
            break; //JPE
        case 0xeb: cycles=5; pair1=state->H; state->H=state->D; state->D=pair1; pair1=state->L; state->L=state->E; state->E=pair1; break; //XCHG
        case 0xec: cycles=17; if(state->cc.p)
        {state->PC+=3; state->memory[state->SP-1]=state->PC>>8; state->memory[state->SP-2]=state->PC; state->SP-=2;
            state->PC=(code[2]<<8)|code[1]; state->PC--; break;} //call
        else
        {
            state->PC+=2;
        }
            break; //CPE addr
        case 0xed: cycles=17; break; //NA
        case 0xee: cycles=7; state->A=(state->A^code[1]);
            state->cc.z=(state->A==0);
            state->cc.s=(bool)(state->A&128);
            state->cc.p=parity(state->A);
            state->cc.cy=0; //CY flag
            state->PC++;
            break; //XRI D8
        case 0xef: cycles=11; state->PC++; state->memory[state->SP-1]=state->PC>>8; state->memory[state->SP-2]=state->PC; state->SP-=2;
            state->PC=40; return cycles; //RST 5 ($28)
        case 0xf0: cycles=11; if(!(state->cc.s))
        {state->PC=((state->memory[state->SP+1]<<8)|state->memory[state->SP]); state->SP+=2; state->PC--;} //return
            break; //RP
        case 0xf1: cycles=10; state->cc.cy=(state->memory[state->SP])&1;
            state->cc.p=((state->memory[state->SP])>>2)&1;
            state->cc.ac=((state->memory[state->SP])>>4)&1;
            state->cc.z=((state->memory[state->SP])>>6)&1;
            state->cc.s=((state->memory[state->SP])>>7)&1;
            state->A=state->memory[state->SP+1];
            state->SP+=2;
            break; //POP PSW
        case 0xf2: cycles=10; if(!(state->cc.s))
        {state->PC=(code[2]<<8)|code[1]; state->PC--;}
        else
        {
            state->PC+=2;
        }
            break; //JP
        case 0xf3: state->int_enable=0; cycles=4; break; //DI SPECIAL
        case 0xf4: cycles=17; if(!(state->cc.s))
        {state->PC+=3; state->memory[state->SP-1]=state->PC>>8; state->memory[state->SP-2]=state->PC; state->SP-=2;
            state->PC=(code[2]<<8)|code[1]; state->PC--; break;} //call
        else
        {
            state->PC+=2;
        }
            break; //CP addr
        case 0xf5: cycles=11; state->memory[state->SP-1]=state->A;
            pair1=state->cc.s; pair1=(pair1<<1)|state->cc.z; pair1=(pair1<<1);/*0*/ pair1=(pair1<<1)|state->cc.ac; pair1=(pair1<<1);/*0*/
            pair1=(pair1<<1)|state->cc.p; pair1=(pair1<<1)|1;/*1*/ pair1=(pair1<<1)|state->cc.cy;
            state->memory[state->SP-2]=pair1; state->SP-=2;
            break; //PUSH PSW FIXED?
        case 0xf6: cycles=7; state->A=(state->A|code[1]);
            state->cc.z=(state->A==0);
            state->cc.s=(bool)(state->A&128);
            state->cc.p=parity(state->A);
            state->cc.cy=0; //CY flag
            state->PC++;
            break; //ORI D8
        case 0xf7: cycles=11; state->PC++; state->memory[state->SP-1]=state->PC>>8; state->memory[state->SP-2]=state->PC; state->SP-=2;
            state->PC=48; return cycles; //RST 6 ($30)
        case 0xf8: cycles=11; if(state->cc.s)
        {state->PC=((state->memory[state->SP+1]<<8)|state->memory[state->SP]); state->SP+=2; state->PC--;} //return
            break; //RM
        case 0xf9: cycles=5; state->SP=(state->H<<8)|state->L; break; //SPHL
        case 0xfa: cycles=10; if(state->cc.s)
        {state->PC=(code[2]<<8)|code[1]; state->PC--;}
        else
        {
            state->PC+=2;
        }
            break; //JM
        case 0xfb: cycles=4; state->int_enable=1; break; //EI
        case 0xfc: cycles=17; if(state->cc.s)
        {state->PC+=3; state->memory[state->SP-1]=state->PC>>8; state->memory[state->SP-2]=state->PC; state->SP-=2;
            state->PC=(code[2]<<8)|code[1]; state->PC--; break;} //call
        else
        {
            state->PC+=2;
        }
            break; //CM addr
        case 0xfd: cycles=17; break; //NA
        case 0xfe: cycles=7; pair1=state->A-code[1];
            state->cc.cy=(state->A<code[1]); //CY flag
            state->cc.z=(state->A==code[1]);
            state->cc.s=(bool)(pair1&128);
            state->cc.p=parity(pair1);
            state->PC++;
            break; //CPI D8
        case 0xff: cycles=11; state->PC++; state->memory[state->SP-1]=state->PC>>8; state->memory[state->SP-2]=state->PC; state->SP-=2;
            state->PC=56; return cycles; //RST 7 ($38)
        default: printf("Unimplemented Code: PC=%02x OP==%04x\n\n\n", state->PC, code[0]);
    }
    state->PC++;
    return cycles;
}
