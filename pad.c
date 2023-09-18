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
int pad_setup(char *, char *);

int sockfd, fdflags;
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
  sockopt.sin_addr.s_addr = inet_addr(inet_ntoa(*(struct in_addr *) resaddr->h_addr));

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
  char *pad_input;

  switch(getchar())
  {
    case 'z':
    {
      char pad_arr[sizeof PAD_PREFIX + sizeof PAD_CROSS - 1];

      if(!pad_setup(PAD_CROSS, pad_arr)) return 0;
      pad_input = pad_arr;
      break;
    }
    case 'a':
    {
      char pad_arr[sizeof PAD_PREFIX + sizeof PAD_SQUARE - 1];

      if(!pad_setup(PAD_SQUARE, pad_arr)) return 0;
      pad_input = pad_arr;
      break;
    }
    case 's':
    {
      char pad_arr[sizeof PAD_PREFIX + sizeof PAD_CIRCLE - 1];

      if(!pad_setup(PAD_CIRCLE, pad_arr)) return 0;
      pad_input = pad_arr;
      break;
    }
    case 'w':
    {
      char pad_arr[sizeof PAD_PREFIX + sizeof PAD_TRIANGLE - 1];

      if(!pad_setup(PAD_TRIANGLE, pad_arr)) return 0;
      pad_input = pad_arr;
      break;
    }
    case 'q':
    {
      char pad_arr[sizeof PAD_PREFIX + sizeof PAD_L1 - 1];

      if(!pad_setup(PAD_L1, pad_arr)) return 0;
      pad_input = pad_arr;
      break;
    }
    case 'e':
    {
      char pad_arr[sizeof PAD_PREFIX + sizeof PAD_R1 - 1];

      if(!pad_setup(PAD_R1, pad_arr)) return 0;
      pad_input = pad_arr;
      break;
    }
    case 'x':
    {
      char pad_arr[sizeof PAD_PREFIX + sizeof PAD_L2 - 1];

      if(!pad_setup(PAD_L2, pad_arr)) return 0;
      pad_input = pad_arr;
      break;
    }
    case 'c':
    {
      char pad_arr[sizeof PAD_PREFIX + sizeof PAD_R2 - 1];

      if(!pad_setup(PAD_R2, pad_arr)) return 0;
      pad_input = pad_arr;
      break;
    }
    case '1':
    {
      char pad_arr[sizeof PAD_PREFIX + sizeof PAD_START - 1];

      if(!pad_setup(PAD_START, pad_arr)) return 0;
      pad_input = pad_arr;
      break;
    }
    case '2':
    {
      char pad_arr[sizeof PAD_PREFIX + sizeof PAD_SELECT - 1];

      if(!pad_setup(PAD_SELECT, pad_arr)) return 0;
      pad_input = pad_arr;
      break;
    }
    case 'h':
    {
      char pad_arr[sizeof PAD_PREFIX + sizeof PAD_HOME - 1];

      if(!pad_setup(PAD_HOME, pad_arr)) return 0;
      pad_input = pad_arr;
      break;
    }
    case 'H':
    {
      char pad_arr[sizeof PAD_PREFIX + sizeof PAD_HOME_HOLD - 1];

      if(!pad_setup(PAD_HOME_HOLD, pad_arr)) return 0;
      pad_input = pad_arr;
      break;
    }
    case 27: /* For arrow keys. */
      if(!fcntl_setup(0)) return 0;
      usleep(1000);
      if(getchar() == '[')
      {
        usleep(1000);
        switch(getchar())
        {
          case 'A':
          {
            char pad_arr[sizeof PAD_PREFIX + sizeof PAD_UP - 1];

            if(!fcntl_setup(1)) return 0;
            else if(!pad_setup(PAD_UP, pad_arr)) return 0;
            pad_input = pad_arr;
            break;
          }
          case 'B':
          {
            char pad_arr[sizeof PAD_PREFIX + sizeof PAD_DOWN - 1];

            if(!fcntl_setup(1)) return 0;
            else if(!pad_setup(PAD_DOWN, pad_arr)) return 0;
            pad_input = pad_arr;
            break;
          }
          case 'C':
          {
            char pad_arr[sizeof PAD_PREFIX + sizeof PAD_RIGHT - 1];

            if(!fcntl_setup(1)) return 0;
            else if(!pad_setup(PAD_RIGHT, pad_arr)) return 0;
            pad_input = pad_arr;
            break;
          }
          case 'D':
          {
            char pad_arr[sizeof PAD_PREFIX + sizeof PAD_LEFT - 1];

            if(!fcntl_setup(1)) return 0;
            else if(!pad_setup(PAD_LEFT, pad_arr)) return 0;
            pad_input = pad_arr;
            break;
          }
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
  write(sockfd, pad_input, strlen(pad_input));
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

int pad_setup(char *padkey, char *pad_input)
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

  memcpy(pad_input, PAD_PREFIX, sizeof PAD_PREFIX - 1);
  strcpy(pad_input + sizeof PAD_PREFIX - 1, padkey);

  return 1;
}
