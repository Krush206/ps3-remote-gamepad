@interface PadSetup: NSObject
{
  NSString *padobj;
  int sockfd;
  struct sockaddr_in sockopt;
}

- (BOOL) connectKey: (NSString *) padkey;
- (BOOL) padConnect;
- (void) padExit;
- (void) setAddress: (const char *) addr;
@end

@interface TtySetup: NSObject
{
  int fdflags;
}

- (BOOL) setStdinFlags: (BOOL) reset;
- (BOOL) makeRawTerm;
- (BOOL) getStdinFlags;
@end
