@interface PadSetup: NSObject
- (BOOL) sendKey: (const char *) padkey keyLength: (NSInteger) key_len;
- (BOOL) padConnect;
- (void) padExit;
@end

@interface TtySetup: NSObject
- (BOOL) stdinFlags: (BOOL) reset;
- (BOOL) termSetup;
@end
