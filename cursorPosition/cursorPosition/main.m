//
//  main.m
//  cursorPosition
//
//  Created by Tomas Vega on 10/6/16.
//  Copyright © 2016 Tomas Vega. All rights reserved.
//

#import <Foundation/Foundation.h>
#include <AppKit/AppKit.h>

@interface Positions : NSObject
@property NSTimer *timer;
-(id)init;
-(void)onTick:(NSTimer *)aTimer;
-(void)printWindow;
@end

@implementation Positions
-(id)init {
    id newInstance = [super init];
    if (newInstance) {
        NSLog(@"Creating timer...");
        _timer = [NSTimer
                    scheduledTimerWithTimeInterval:1.0
                    target:self
                    selector:@selector(onTick:)
                    userInfo:nil
                    repeats:YES];
    }
    return newInstance;
}

-(NSObject *)getWindowPosition {
    NSMutableArray *windows = (__bridge NSMutableArray *)CGWindowListCopyWindowInfo(kCGWindowListOptionOnScreenOnly | kCGWindowListExcludeDesktopElements, kCGNullWindowID);
    
    NSPredicate *predicate = [NSPredicate predicateWithFormat:@"kCGWindowName == %@", @"Minesweeper Deluxe"];
    NSArray *filteredArray = [windows filteredArrayUsingPredicate:predicate];
    if ([filteredArray count]) {
        return filteredArray[0][@"kCGWindowBounds"];
    } else {
        NSLog(@"Open Minesweeper!");
        return nil;
    }
}

-(NSArray *)getCursorPosition {
    NSPoint mouseLoc = [NSEvent mouseLocation]; //get current mouse position
    NSNumber *xPos = [NSNumber numberWithDouble:mouseLoc.x];
    NSNumber *yPos = [NSNumber numberWithDouble:mouseLoc.y];
    NSArray *cursorPos = @[xPos, yPos];
    return cursorPos;
}

-(void)onTick:(NSTimer *)aTimer {

    NSObject *windowPos = [self getWindowPosition];
    NSObject *cursorPos = [self getCursorPosition];
    NSLog(@"%@", windowPos);
    NSLog(@"%@", cursorPos);

}

@end

int main() {
    @autoreleasepool {
        Positions *posObj = [[Positions alloc] init];
        [[NSRunLoop currentRunLoop] run];
    }


    return 0;
}