//
//  SpaceInvadersMachine.h
//  Space Invaders
//
//  Created by Holden Scholl on 8/4/19.
//  Copyright © 2019 Holden. All rights reserved.
//

#ifndef SpaceInvadersMachine_h
#define SpaceInvadersMachine_h

#import <Foundation/Foundation.h>
//#import <AVFoundation/AVFoundation.h>
#import <AppKit/AppKit.h>
#include "shellhead.hpp"

using namespace SHELL;

@interface SpaceInvadersMachine : NSObject
{
    State8080   *state;
    Machine *machine;
    
    uint8_t oldPort3;
    uint8_t oldPort5;
    double      lastTimer;
    double      nextInterrupt;
    int         whichInterrupt;
    NSTimer     *emulatorTimer;
    NSSound *ufo;
}

-(double) microseconds;

-(id) init;

-(void) doCPU;
-(void) startEmulation;
-(void) keyUp: (uint8_t) key;
-(void) keyDown: (uint8_t) key;
-(void) playSounds;
-(uint8_t *) framebuffer;


@end

#endif /* spaceinvadersmachine_hpp */

