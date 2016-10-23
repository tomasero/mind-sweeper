//
//  main.m
//  cursorPosition
//
//  Created by Tomas Vega on 10/6/16.
//  Copyright © 2016 Tomas Vega. All rights reserved.
//

#import <Foundation/Foundation.h>
#include <AppKit/AppKit.h>
#import "main.h"
#include <sys/ioctl.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/serial/IOSerialKeys.h>
#include <IOKit/IOBSD.h>

@interface Positions : NSObject
@property NSTimer *timer;
-(id)init;
-(void)onTick:(NSTimer *)aTimer;
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

-(NSDictionary *)getWindowPosition {
    NSMutableArray *windows = (__bridge NSMutableArray *)CGWindowListCopyWindowInfo(kCGWindowListOptionOnScreenOnly | kCGWindowListExcludeDesktopElements, kCGNullWindowID);
    
    NSPredicate *predicate = [NSPredicate predicateWithFormat:@"kCGWindowName == %@", @"Minesweeper Deluxe"];
    NSArray *filteredArray = [windows filteredArrayUsingPredicate:predicate];
    if ([filteredArray count]) {
        return filteredArray[0][@"kCGWindowBounds"];
    } else {
        return nil;
    }
}

-(NSDictionary *)getCursorPosition {
    NSPoint mouseLoc = [NSEvent mouseLocation]; //get current mouse position
    NSRect screen = [[NSScreen mainScreen] frame];
    int y = screen.size.height - mouseLoc.y;
    NSDictionary *cursorPos = @{
        @"X": [NSNumber numberWithInt:mouseLoc.x],
        @"Y": [NSNumber numberWithInt:y]
    };
    return cursorPos;
}

-(bool)cursor:(NSDictionary *)cursorPos isInsideWindow:(NSDictionary *)windowPos {
    int cursorX = [cursorPos[@"X"] intValue];
    int cursorY = [cursorPos[@"Y"] intValue];
    int windowX = [windowPos[@"X"] intValue];
    int windowY = [windowPos[@"Y"] intValue];
    int windowWidth = [windowPos[@"Width"] intValue];
    int windowHeight = [windowPos[@"Height"] intValue];
    bool isX = cursorX > windowX && cursorX < (windowX + windowWidth);
    bool isY = cursorY > windowY && cursorY < (windowY + windowHeight);
    return isX && isY;
}

-(NSDictionary *)getCursorPosition:(NSDictionary *)cursorPos relativeToWindow:(NSDictionary *)windowPos {
    int cursorX = [cursorPos[@"X"] intValue];
    int cursorY = [cursorPos[@"Y"] intValue];
    int windowX = [windowPos[@"X"] intValue];
    int windowY = [windowPos[@"Y"] intValue];
    NSDictionary *relPos = @{
        @"X": [NSNumber numberWithInt:cursorX - windowX],
        @"Y": [NSNumber numberWithInt:cursorY - windowY]
    };
    return relPos;
}

-(NSDictionary *)getCursorRelativePosition:(NSDictionary *)cursorPos {
    int cursorX = [cursorPos[@"X"] intValue];
    int cursorY = [cursorPos[@"Y"] intValue];
    int windowX = 15;
    int windowY = 98;
    int relX = cursorX - windowX;
    int relY = cursorY - windowY;
    NSDictionary *relPos = @{
         @"X": [NSNumber numberWithInt:cursorX - windowX],
         @"Y": [NSNumber numberWithInt:cursorY - windowY]
     };
    return relPos;
}

-(NSDictionary *)getCell:(NSDictionary *)cursorPos {
    int cursorX = [cursorPos[@"X"] intValue];
    int cursorY = [cursorPos[@"Y"] intValue];
    if ((cursorX < 0 || cursorX > 479) || (cursorY < 0 || cursorY > 479)) {
        return nil;
    }
    int cellX = cursorX / 30;
    int cellY = cursorY / 30;
    
//    NSLog(@"(%d, %d)", cellX, cellY);
    NSDictionary *cell = @{
        @"X": [NSNumber numberWithInt: cellX],
        @"Y": [NSNumber numberWithInt: cellY]
    };
    return cell;
}

-(void)sendCellThroughSerial:(NSDictionary *)cell {
    unsigned char data[2];
    data[1] = (char)[self getDangerMetric: cell];
    data[0]=0xff;
    write(serialFileDescriptor, &data, 2);
//    close(serialFileDescriptor);
}

-(void)sendNothingThroughSerial {
    unsigned char data[2];
    data[1] = 0xfe;
    data[0]=0xff;
    write(serialFileDescriptor, &data, 2);
    //    close(serialFileDescriptor);
}

-(int)getDangerMetric:(NSDictionary *)cell {
    int x = [cell[@"X"] intValue];
    int y = [cell[@"Y"] intValue];
    int danger = heatMap[x][y];
    NSLog(@"danger: %d", danger);
    return danger;
}

-(void)printRelPos:(NSDictionary *)windowPos cursor:(NSDictionary *)cursorPos {
    if ([self cursor:cursorPos isInsideWindow:windowPos]) {
        NSLog(@"Inside!");
        NSDictionary *windowRelPos = [self getCursorPosition:cursorPos relativeToWindow:windowPos];
        NSDictionary *gameRelPos = [self getCursorRelativePosition: windowRelPos];
        NSDictionary *cell = [self getCell:gameRelPos];
        if (cell != nil) {
            [self sendCellThroughSerial:cell];
        } else {
            NSLog(@"Outside tiles");
            [self sendNothingThroughSerial];
        }
    } else {
        NSLog(@"Outside Minesweeper!");
        [self sendNothingThroughSerial];
    }
}

-(void)onTick:(NSTimer *)timer {
    [self readFile];
    NSDictionary *windowPos = [self getWindowPosition];
    NSDictionary *cursorPos = [self getCursorPosition];
//    NSLog(@"%@", windowPos);
//    NSLog(@"%@", cursorPos);
    if (windowPos) {
        [self printRelPos:windowPos cursor:cursorPos];
    } else {
        NSLog(@"Open Minesweeper!");
    }
}

- (void) openSerial {
    serialFileDescriptor = open(
        "/dev/cu.usbmodem1267101",
//        "/dev/cu.usbmodem1451",
        O_RDWR |
        O_NOCTTY |
        O_NONBLOCK
    );
    
    if (serialFileDescriptor == -1)
    {
        printf("Error opening serial port");
    }
    
    // block non-root users from using this port
//    ioctl(serialFileDescriptor, TIOCEXCL);
    
    // clear the O_NONBLOCK flag, so that read() will
    //   block and wait for data.
    fcntl(serialFileDescriptor, F_SETFL, 0);
    
    // grab the options for the serial port
    tcgetattr(serialFileDescriptor, &options);
    
    // setting raw-mode allows the use of tcsetattr() and ioctl()
    cfmakeraw(&options);
    
    // specify any arbitrary baud rate
    ioctl(serialFileDescriptor, IOSSIOSPEED, B9600);
}

- (void) testArray {
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 16; j++) {
            int val = 16 * i + j;
            heatMap[j][i] = val < 255 ?  val : 254;
//            NSLog(@"%d", heatMap[j][i]);
        }
    }
}

//- (void) readFile {
////    NSString *filepath = [[NSBundle mainBundle] pathForResource:@"hardcode" ofType:@"txt"];
////    NSError *error;
////    NSString *fileContents = [NSString stringWithContentsOfFile:filepath encoding:NSUTF8StringEncoding error:&error];
//
//    NSFileManager *filemgr;
//    NSData *databuffer;
//    
//    filemgr = [NSFileManager defaultManager];
//    
//    databuffer = [filemgr contentsAtPath: @"hardcode.txt"];
//    
////    if (error)
////        NSLog(@"Error reading file: %@", error.localizedDescription);
//    
//    // maybe for debugging...
////    NSLog(@"contents: %@", fileContents);
//    
////    NSArray *listArray = [fileContents componentsSeparatedByString:@"\n"];
//    NSString* fileContents = [NSString stringWithUTF8String:[databuffer bytes]];
//    NSLog(@"%@", fileContents);
//    NSArray *listArray = [fileContents componentsSeparatedByString:@"\n"];
//    
//    for (int i = 0; i < 16; i++) {
//        for (int j = 0; j < 16; j++) {
//            int idx = 16 * i + j;
//            heatMap[j][i] = [(NSNumber *)[listArray objectAtIndex:idx] intValue];
//        }
//    }
//}

- (void) readFile {
    FILE *fp;
    long lSize;
    char *buffer;
    
    fp = fopen ( "/Users/cortensinger/Research/mindSweeper/cursorPosition/hardcode.txt" , "rb" );
    if( !fp ) perror("/Users/cortensinger/Research/mindSweeper/cursorPosition/hardcode.txt"),exit(1);
    
    fseek( fp , 0L , SEEK_END);
    lSize = ftell( fp );
    rewind( fp );
    
    /* allocate memory for entire content */
    buffer = calloc( 1, lSize+1 );
    if( !buffer ) fclose(fp),fputs("memory alloc fails",stderr),exit(1);
    
    /* copy the file into the buffer */
    if( 1!=fread( buffer , lSize, 1 , fp) )
        fclose(fp),free(buffer),fputs("entire read fails",stderr),exit(1);
    

    NSString *fileContents = [NSString stringWithCString:buffer encoding:NSASCIIStringEncoding];

//    NSLog(@"%@", fileContents);
    NSArray *listArray = [fileContents componentsSeparatedByString:@"\n"];

    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 16; j++) {
            int idx = 16 * i + j;
            heatMap[j][i] = [(NSNumber *)[listArray objectAtIndex:idx] intValue];
        }
    }
    fclose(fp);
    free(buffer);
}

@end


int main() {
    @autoreleasepool {
        Positions *posObj = [[Positions alloc] init];
//        [posObj readFile];
        serialFileDescriptor = -1;
        [posObj openSerial];
        [[NSRunLoop currentRunLoop] run];
    }


    return 0;
}