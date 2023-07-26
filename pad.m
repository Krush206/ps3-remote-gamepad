#import <Foundation/Foundation.h>
#import <unistd.h>
#import <termios.h>
#import <arpa/inet.h>
#import <netdb.h>
#import "pad.h"

#define PAD_PREFIX @"GET /pad.ps3?"

int sockfd, fdflags, input_len;
struct sockaddr_in sockopt;

@implementation NSString (getchar)
- (NSMutableString *) getArrowKey: (BOOL) is_arrow
{
  char c;
  NSMutableString *key = [NSMutableString new];
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
      [key appendFormat: @"%c", c];
      if(is_arrow) return key;
      else if(!keysdict[key]) return nil;
  }

  return key;
}

- (NSMutableString *) parseArrowKey
{
  NSMutableString *key;
  NSDictionary *keysdict = @{
    @"A": @"up",
    @"B": @"down",
    @"C": @"right",
    @"D": @"left",
  };
  TtySetup *ttySetup = [TtySetup new];

  [ttySetup resetStdinFlags: NO];
  usleep(1000);
  if([[self getArrowKey: YES] isEqualToString: @"["])
  {
    usleep(1000);
    key = [self getArrowKey: YES];
    [ttySetup resetStdinFlags: YES];
    return keysdict[key];
  }
      
  return nil;
}
@end

@implementation PadSetup
{
  NSMutableString *padobj;
}

- (BOOL) sendKey: (const char *) padkey keyLength: (int) key_len
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

  padobj = [NSMutableString new];
  [padobj appendString: PAD_PREFIX];
  [padobj appendFormat: @"%s", padkey];

  return YES;
}

- (BOOL) padConnect
{
  NSString *key = [NSString new];
  NSMutableString *input = [NSMutableString new];
  const char *pad_input;

  if(![input getArrowKey: NO]) return YES;
  else if(![self sendKey: [key UTF8String] keyLength: [key length]]) return NO;

  printf("%s\r\n", pad_input = [padobj UTF8String]);
  write(sockfd, pad_input, input_len);
  close(sockfd);

  return YES;
}

- (void) padExit
{
  printf("Exiting...\r\n");
  exit(0);
}
@end

@implementation TtySetup
- (BOOL) resetStdinFlags: (BOOL) reset
{
  if(reset)
  {
    if(fcntl(0, F_SETFL, fdflags) < 0)
    {
      fprintf(stderr, "Failed to reset stdin descriptor flags.\r\n");
      return NO;
    }
  }
  else
    if(fcntl(0, F_SETFL, fdflags | O_NONBLOCK) < 0)
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
@end

int main(int argc, char **argv)
{
  struct hostent *resaddr;
  NSAutoreleasePool *pool = [NSAutoreleasePool new];
  PadSetup *padSetup = [PadSetup new];
  TtySetup *ttySetup = [TtySetup new];

  if(argc < 2 || argc > 2)
  {
    fprintf(stderr, "Usage: ./a.out <PlayStation 3 ip>\n");
    return -1;
  }
  else if((resaddr = gethostbyname(argv[1])) == NULL)
  {
    fprintf(stderr, "Invalid address.\n");
    return -1;
  }
  
  sockopt.sin_family = AF_INET;
  sockopt.sin_port = htons(80);
  inet_aton(argv[1], &sockopt.sin_addr);

  if(![ttySetup makeRawTerm])
  {
    fprintf(stderr, "Failed to set up the terminal.\n");
    return -1;
  }
  else if((fdflags = fcntl(0, F_GETFL)) < 0)
  {
    fprintf(stderr, "Failed to get stdin descriptor flags.\r\n");
    return -1;
  }

  while(1)
    if(![padSetup padConnect]) return -1;

  [pool drain];
}
