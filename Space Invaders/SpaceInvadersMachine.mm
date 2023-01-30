//
//  SpaceInvadersMachine.m
//  Space Invaders
//
//  Created by Holden Scholl on 8/4/19.
//  Copyright © 2019 Holden. All rights reserved.
//

#include "SpaceInvadersMachine.h"
extern "C"{
#include <sys/time.h>
}

using namespace SHELL;

@implementation SpaceInvadersMachine

-(id) init
{
    //State8080 * Machine * 
    state = new State8080;
    machine = new Machine;
    oldPort3=0;
    oldPort5=0;
    state->memory = new uint8_t[65535]();
    
    //init bundle, access file, copy file to buffer
    NSBundle * mainBundle = [NSBundle mainBundle];
    NSString * filePath = [mainBundle pathForResource:@"invaders" ofType:NULL];
    NSData * file = [NSData dataWithContentsOfFile:filePath];
    [file getBytes:state->memory length:[file length]];

    return self;
}


-(uint8_t *) framebuffer
{
    return &state->memory[0x2400];
}

-(double) microseconds
{
    struct timeval time;
    gettimeofday(&time, NULL);
    return ((double)time.tv_sec * 1E6) + ((double)time.tv_usec);
}


-(void) doCPU
{
    double now = [self microseconds];
    
    if (lastTimer == 0.0)//init schedule next int for 1/60 of a sec
    {
        lastTimer = now;
        nextInterrupt = lastTimer + 16000.0;
        whichInterrupt = 1;
    }
    if ((state->int_enable>0) && (now > nextInterrupt))
    {
        if (whichInterrupt == 1)
        {
            generateInt(state, 1);
            whichInterrupt = 2;
        }
        else
        {
            generateInt(state, 2);
            whichInterrupt = 1;
        }
        nextInterrupt = now+8000.0;
    }
    
    
    //How much time has passed?  How many instructions will it take to keep up with
    // the current time?  Assume:
    //CPU is 2 MHz
    // so 2M cycles/sec
    
    double sinceLast = now - lastTimer;
    int cycles_to_catch_up = 2 * sinceLast;
    int cycles = 0;
    
    while (cycles_to_catch_up > cycles)
    {
        cycles += emulate8080(state, machine);
        [self playSounds];
    }
    lastTimer  = now;
}

- (void) startEmulation
{
    emulatorTimer = [NSTimer scheduledTimerWithTimeInterval: 0.001
                                                     target: self
                                                   selector:@selector(doCPU)
                                                   userInfo: nil repeats:YES];
}

-(void) keyUp: (uint8_t) key
{
    switch(key)
    {
        case(2): machine->in1 ^= 0b00100000; break; //bit 5 P1 LEFT(<)
        case(3): machine->in1 ^= 0b01000000; break; //bit 6 P1 RIGHT(>)
        case(32): machine->in1 ^= 0b00010000; break; //bit 4 P1 SHOOT(SPACE)
        case(13): machine->in1 ^= 0b00000100; break; //bit 6 P1 START(ENTER)
        case(99): machine->in1 ^= 0b00000001; break; //bit 6 COIN (C)
    }
}

-(void) keyDown: (uint8_t) key
{
    switch(key)
    {
        case(2): machine->in1 |= 0b00100000; break; //bit 5 P1 LEFT(<)
        case(3): machine->in1 |= 0b01000000; break; //bit 6 P1 RIGHT(>)
        case(32): machine->in1 |= 0b00010000; break; //bit 4 P1 SHOOT(SPACE)
        case(13): machine->in1 |= 0b00000100; break; //bit 6 P1 START(ENTER)
        case(99): machine->in1 |= 0b00000001; break; //bit 6 COIN (C)
    }
}

-(void) playSounds
{
    if(machine->out3!=oldPort3)
    {
        if((machine->out3&1)&&!(oldPort3&1))
        {
            //ufo
            ufo = [NSSound soundNamed: @"0.wav"];
            [ufo setLoops:true];
            [ufo play];
        }
        if(!(machine->out3&1)&&(oldPort3&1))
        {
            //ufo
            [ufo stop];
            ufo=NULL;
        }
        if((machine->out3&2)&&!(oldPort3&2))
        {
            //shot
            [[NSSound soundNamed: @"1.wav"] play];
        }
        if((machine->out3&4)&&!(oldPort3&4))
        {
            //player die
            [[NSSound soundNamed: @"2.wav"] play];
        }
        if((machine->out3&8)&&!(oldPort3&8))
        {
            //invader die
            [[NSSound soundNamed: @"3.wav"] play];
        }
        oldPort3=machine->out3;
    }
    if(machine->out5!=oldPort5)
    {
        if((machine->out5&1)&&!(oldPort5&1))
        {
            //fleet move1
            [[NSSound soundNamed: @"4.wav"] play];
        }
        if((machine->out5&2)&&!(oldPort5&2))
        {
            //fleet move2
            [[NSSound soundNamed: @"5.wav"] play];
        }
        if((machine->out5&4)&&!(oldPort5&4))
        {
            //fleet move3
            [[NSSound soundNamed: @"6.wav"] play];
        }
        if((machine->out5&8)&&!(oldPort5&1))
        {
            //fleet move4
            [[NSSound soundNamed: @"7.wav"] play];
        }
        if((machine->out5&16)&&!(oldPort5&1))
        {
            //ufo hit
            [[NSSound soundNamed: @"8.wav"] play];
        }
        oldPort5=machine->out5;
    }
}

@end
