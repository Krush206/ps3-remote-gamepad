#import <Foundation/Foundation.h>
#import <unistd.h>
#import <termios.h>
#import <arpa/inet.h>
#import <netdb.h>
#import <fcntl.h>
#import "Pad.h"

#define PAD_PREFIX @"GET /pad.ps3?"

@implementation NSString (getchar)
- (NSString *) getKey: (BOOL) isArrow
{
  int c;
  NSString *key;
  NSDictionary *keysdict = @{
    @"h": @"_psbtn_home",
    @"H": @"_psbtn_home_hold",
    @"z": @"cross",
    @"s": @"circle",
    @"w": @"triangle",
    @"a": @"square",
    @"q": @"l1",
    @"e": @"r1",
    @"x": @"l2",
    @"c": @"r2",
    @"1": @"start",
    @"2": @"select",
  };
  PadSetup *exit = [PadSetup new];

  switch((c = getchar()))
  {
    case 3: [exit padExit];
    case 27: return [self parseArrowKey];
    default:
      key = [[NSString alloc] initWithFormat: @"%c", c];
      if(isArrow) return key;
      return [keysdict objectForKey: key];
  }

  return nil;
}

- (NSString *) parseArrowKey
{
  NSString *key;
  NSDictionary *keysdict = @{
    @"A": @"up",
    @"B": @"down",
    @"C": @"right",
    @"D": @"left",
  };
  TtySetup *ttySetup = [TtySetup new];

  [ttySetup setStdinFlags: NO];
  usleep(1000);
  if([[self getKey: YES] isEqualToString: @"["])
  {
    usleep(1000);
    key = [self getKey: YES];
    [ttySetup setStdinFlags: YES];
    return [keysdict objectForKey: key];
  }
      
  return nil;
}
@end

@implementation PadSetup
- (BOOL) connectKey: (NSString *) padkey
{
  if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    fprintf(stderr, "Failed to open socket.\r\n");
    return NO;
  }
  else if(connect(sockfd, (struct sockaddr *) &sockopt, sizeof sockopt) < 0)
  {
    fprintf(stderr, "Failed to connect.\r\n");
    return NO;
  }

  padobj = [[NSString alloc] initWithFormat: @"%@%@", PAD_PREFIX, padkey];

  return YES;
}

- (BOOL) padConnect
{
  NSString *input = [NSString new];
  const char *pad_input;

  if(!(input = [input getKey: NO])) return YES;
  else if(![self connectKey: input]) return NO;

  printf("%s\r\n", pad_input = [padobj UTF8String]);
  write(sockfd, pad_input, [padobj length]);
  close(sockfd);

  return YES;
}

- (void) padExit
{
  printf("Exiting...\r\n");
  exit(0);
}

- (void) setAddress: (const char *) addr
{
  sockopt.sin_family = AF_INET;
  sockopt.sin_port = htons(80);
  sockopt.sin_addr.s_addr = inet_addr(inet_ntoa(*(struct in_addr *) addr));
}
@end

@implementation TtySetup
- (BOOL) setStdinFlags: (BOOL) reset
{
  if(reset)
  {
    if(fcntl(0, F_SETFL, fdflags) < 0)
    {
      fprintf(stderr, "Failed to reset stdin descriptor flags.\r\n");
      return NO;
    }
  }
  else if(fcntl(0, F_SETFL, fdflags | O_NONBLOCK) < 0)
  {
    fprintf(stderr, "Failed to set stdin descriptor flags.\r\n");
    return NO;
  }

  return YES;
}

- (BOOL) makeRawTerm
{
  struct termios term;

  if(tcgetattr(0, &term) < 0) return NO;
  cfmakeraw(&term);
  if(tcsetattr(0, TCSANOW, &term) < 0) return NO;

  return YES;
}

- (BOOL) getStdinFlags
{
  if((fdflags = fcntl(0, F_GETFL)) < 0)
    return NO;

  return YES;
}
@end

int main(int argc, char **argv)
{
  @autoreleasepool
  {
    struct hostent *resaddr;
    PadSetup *padSetup = [PadSetup new];
    TtySetup *ttySetup = [TtySetup new];
  
    if(argc < 2 || argc > 2)
    {
      fprintf(stderr, "Usage: ./a.out <PlayStation 3 ip>\n");
      return -1;
    }
    else if(!(resaddr = gethostbyname(argv[1])))
    {
      fprintf(stderr, "Invalid address.\n");
      return -1;
    }
    
    if(![ttySetup makeRawTerm])
    {
      fprintf(stderr, "Failed to set up the terminal.\n");
      return -1;
    }
    else if(![ttySetup getStdinFlags])
    {
      fprintf(stderr, "Failed to get stdin descriptor flags.\r\n");
      return -1;
    }
  
    [padSetup setAddress: resaddr->h_addr];
    while(YES)
      if(![padSetup padConnect]) return -1;
  }
}
