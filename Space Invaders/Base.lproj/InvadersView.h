//
//  InvadersView.h
//  Space Invaders
//
//  Created by Holden Scholl on 8/4/19.
//  Copyright © 2019 Holden. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "SpaceInvadersMachine.h"

NS_ASSUME_NONNULL_BEGIN

@interface InvadersView : NSView
{
    NSTimer *renderTimer;
    SpaceInvadersMachine *invaders;
    uint16_t * videoBuffer;
    
    CGContextRef bmpContext;
}

- (void)timerFired:(id)sender;

- (void)keyDown:(NSEvent *)theEvent;
- (void)keyUp:(NSEvent *)theEvent;

@end

NS_ASSUME_NONNULL_END
