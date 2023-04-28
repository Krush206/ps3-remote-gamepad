#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <termios.h>
#include <fcntl.h>

#define PAD_PREFIX "GET /pad.ps3?"
#define PAD_HOME "_psbtn_home"
#define PAD_HOME_HOLD "_psbtn_home_hold"
#define PAD_UP "up"
#define PAD_DOWN "down"
#define PAD_RIGHT "right"
#define PAD_LEFT "left"
#define PAD_CROSS "cross"
#define PAD_CIRCLE "circle"
#define PAD_TRIANGLE "triangle"
#define PAD_SQUARE "square"
#define PAD_L1 "l1"
#define PAD_R1 "r1"
#define PAD_L2 "l2"
#define PAD_R2 "r2"
#define PAD_START "start"
#define PAD_SELECT "select"

int pad_connect(void);
int getkey(void);
int fcntl_setup(int);
int pad_setup(char *, int);

int sockfd, fdflags, input_len;
char *pad_input;
struct sockaddr_in sockopt;

int main(int argc, char **argv)
{
  struct hostent *resaddr;

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
  inet_aton(*resaddr->h_addr_list, &sockopt.sin_addr);

  if(!getkey())
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
    if(!pad_connect()) return -1;
}

int pad_connect(void)
{
  switch(getchar())
  {
    case 'z':
      if(!pad_setup(PAD_CROSS, sizeof PAD_CROSS)) return 0;
      break;
    case 'a':
      if(!pad_setup(PAD_SQUARE, sizeof PAD_SQUARE)) return 0;
      break;
    case 's':
      if(!pad_setup(PAD_CIRCLE, sizeof PAD_CIRCLE)) return 0;
      break;
    case 'w':
      if(!pad_setup(PAD_TRIANGLE, sizeof PAD_TRIANGLE)) return 0;
      break;
    case 'q':
      if(!pad_setup(PAD_L1, sizeof PAD_L1)) return 0;
      break;
    case 'e':
      if(!pad_setup(PAD_R1, sizeof PAD_R1)) return 0;
      break;
    case 'x':
      if(!pad_setup(PAD_L2, sizeof PAD_L2)) return 0;
      break;
    case 'c':
      if(!pad_setup(PAD_R2, sizeof PAD_R2)) return 0;
      break;
    case '1':
      if(!pad_setup(PAD_START, sizeof PAD_START)) return 0;
      break;
    case '2':
      if(!pad_setup(PAD_SELECT, sizeof PAD_SELECT)) return 0;
      break;
    case 'h':
      if(!pad_setup(PAD_HOME, sizeof PAD_HOME)) return 0;
      break;
    case 'H':
      if(!pad_setup(PAD_HOME_HOLD, sizeof PAD_HOME_HOLD)) return 0;
      break;
    case 27: /* For arrow keys. */
      if(!fcntl_setup(0)) return 0;
      usleep(1000);
      if(getchar() == '[')
      {
        usleep(1000);
        switch(getchar())
        {
          case 'A':
            if(!fcntl_setup(1)) return 0;
            else if(!pad_setup(PAD_UP, sizeof PAD_UP)) return 0;
            break;
          case 'B':
            if(!fcntl_setup(1)) return 0;
            else if(!pad_setup(PAD_DOWN, sizeof PAD_DOWN)) return 0;
            break;
          case 'C':
            if(!fcntl_setup(1)) return 0;
            else if(!pad_setup(PAD_RIGHT, sizeof PAD_RIGHT)) return 0;
            break;
          case 'D':
            if(!fcntl_setup(1)) return 0;
            else if(!pad_setup(PAD_LEFT, sizeof PAD_LEFT)) return 0;
            break;
          default:
            if(!fcntl_setup(1)) return 0;
	    return 1;
        }
      }
      else
      {
        if(!fcntl_setup(1)) return 0;
        return 1;
      }
      break;
    case 3: /* For Ctrl+C. */
      printf("Exiting...\r\n");
      exit(0);
    default:
      return 1;
  }

  printf("%s\r\n", pad_input);
  write(sockfd, pad_input, input_len);
  free(pad_input);
  close(sockfd);

  return 1;
}

int getkey(void)
{
  struct termios term;

  if(tcgetattr(0, &term) < 0) return 0;
  cfmakeraw(&term);
  if(tcsetattr(0, TCSANOW, &term) < 0) return 0;

  return 1;
}

int fcntl_setup(int reset)
{
  if(reset)
  {
    if(fcntl(0, F_SETFL, fdflags) < 0)
    {
      fprintf(stderr, "Failed to reset stdin descriptor flags.\r\n");
      return 0;
    }
  }
  else
    if(fcntl(0, F_SETFL, fdflags | O_NONBLOCK) < 0)
    {
      fprintf(stderr, "Failed to set stdin descriptor flags.\r\n");
      return 0;
    }

  return 1;
}

int pad_setup(char *padkey, int key_len)
{
  if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    fprintf(stderr, "Failed to open socket.\r\n");
    return 0;
  }
  else if(connect(sockfd, (struct sockaddr *) &sockopt, sizeof sockopt) < 0)
  {
    fprintf(stderr, "Failed to connect.\r\n");
    return 0;
  }
  else if((pad_input = (char *) malloc(input_len = sizeof PAD_PREFIX + key_len - 1)) == NULL)
  {
    fprintf(stderr, "Failed to allocate memory.\r\n");
    close(sockfd);
    return 0;
  }

  memcpy(pad_input, PAD_PREFIX, sizeof PAD_PREFIX - 1);
  strcpy(pad_input + sizeof PAD_PREFIX - 1, padkey);

  return 1;
}
