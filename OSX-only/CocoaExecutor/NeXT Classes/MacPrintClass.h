#import <Cocoa/Cocoa.h>

@interface MacPrintClass : NSView
{
}

- drawSelf:(NXRect *)rects :(int)rectCount ;
- (BOOL)getRect:(NXRect *)theRect forPage:(int)page;
- (BOOL)knowsPagesFirst:(int *)firstPageNum last:(int *)lastPageNum;
- beginPageSetupRect:(const NXRect  * )aRect 
					  placement:(const NXPoint *) location;
- beginSetup;
- endPage;
- endPSOutput;

@end
