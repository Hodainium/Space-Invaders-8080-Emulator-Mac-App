//
//  InvadersView.m
//  Space Invaders
//
//  Created by Holden Scholl on 8/4/19.
//  Copyright © 2019 Holden. All rights reserved.
//

#import "InvadersView.h"
#include <math.h>
#define WHITE 0xFFFF
#define BLACK 0x0000

@implementation InvadersView

- (void)awakeFromNib
{
    invaders = [[SpaceInvadersMachine alloc] init];
    
    CGColorSpaceRef colorSpace = CGColorSpaceCreateWithName(kCGColorSpaceGenericRGB);
    videoBuffer = new uint16_t[57344];
    bmpContext = CGBitmapContextCreate(videoBuffer, 256, 224, 5, 512, colorSpace, kCGImageAlphaNoneSkipFirst);
    
    //a 16ms time interval to get 60 fps
    renderTimer = [NSTimer timerWithTimeInterval:0.016
                                          target:self
                                        selector:@selector(timerFired:)
                                        userInfo:nil
                                         repeats:YES];
    
    [[NSRunLoop currentRunLoop] addTimer:renderTimer forMode:NSDefaultRunLoopMode];
    [invaders startEmulation];
}

- (void)drawRect:(NSRect)dirtyRect {
    [super drawRect:dirtyRect];
    // Drawing code here.
    {
        uint8_t *framebuffer = [invaders framebuffer];
        for(int i = 0; i < 224; i++)//every row
        {
            for(int j = 0; j < 32; j++)//every byte in column 32 bytes in 256 bits
            {
                int offset = (j*8)+(i*256); //works for video
                uint8_t byte = framebuffer[offset/8];
                for(int b = 0; b < 8; b++) //every bit in byte
                {
                    if((bool)(byte&(1<<b))) //checking every bit from LSB to MSB
                    {
                        //pixel ON
                        videoBuffer[offset+b]=WHITE;
                    }
                    else
                    {
                        //pixel OFF
                        videoBuffer[offset+b]=BLACK;
                    }
                }
            }
        }
        
    }
    CGImageRef screen = CGBitmapContextCreateImage(bmpContext);
    CGContextRef context = [[NSGraphicsContext currentContext] CGContext];
    CGContextRotateCTM(context, (90.0*M_PI/180)); //radians(90)
    CGContextTranslateCTM(context, 0, -224);
    CGContextDrawImage(context, CGRectMake(0, 0, 256, 224), screen);
    CGImageRelease(screen);
}

- (void)timerFired:(id)sender
{
    [self setNeedsDisplay:YES];
}

- (BOOL)acceptsFirstResponder {
    return YES;
}

- (void)keyDown:(NSEvent *)theEvent {
    NSString *chars = theEvent.charactersIgnoringModifiers;
    [invaders keyDown:([chars characterAtIndex: 0])];
}

- (void)keyUp:(NSEvent *)theEvent {
    NSString *chars = theEvent.charactersIgnoringModifiers;
    [invaders keyUp:([chars characterAtIndex: 0])];
}

@end
