//
//  shell.h
//  Space Invaders
//
//  Created by Holden Scholl on 8/10/19.
//  Copyright © 2019 Holden. All rights reserved.
//

#ifndef shellhead_hpp
#define shellhead_hpp

extern "C"{
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
}

namespace SHELL
{
    
    struct ShiftPort
    {
        uint16_t shiftReg;
        ShiftPort();
    };
    
    struct Machine
    {
        uint8_t in1;
        uint8_t in2;
        uint8_t in3;
        uint8_t out2;
        uint8_t out3;
        uint8_t out4;
        uint8_t out5;
        ShiftPort shift;
        uint8_t readShift();
        void writeShift();
        Machine();
    };
    
    struct ConditionCodes
    {
        uint8_t z:1;
        uint8_t s:1;
        uint8_t p:1;
        uint8_t ac:1;
        uint8_t cy:1;
        uint8_t pad:3;
        ConditionCodes();
    };
    
    struct State8080
    {
        uint8_t A;
        uint8_t B;
        uint8_t C;
        uint8_t D;
        uint8_t E;
        uint8_t H;
        uint8_t L;
        uint8_t int_enable;
        uint16_t SP;
        uint16_t PC;
        uint8_t * memory;
        ConditionCodes cc;
        State8080();
    };
    
    int parity(int x, int size=8);
    void emulateMachine(Machine * machine);
    void generateInt(State8080 * state, int interrupt);
    int emulate8080(State8080 * state, Machine * machine);
}

#endif /* shell_hpp */
