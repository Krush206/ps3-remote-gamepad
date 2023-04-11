#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <termios.h>

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

#define pad_setup(padkey) \
        pad_input = malloc(input_len = sizeof PAD_PREFIX + sizeof padkey - 1); \
        memcpy(pad_input, PAD_PREFIX, sizeof PAD_PREFIX - 1); \
        strcpy(pad_input + sizeof PAD_PREFIX - 1, padkey)

int pad_connect(void);
int getkey(void);

int sockfd;

int main(int argc, char **argv)
{
  int ret;
  struct sockaddr_in sockopt;

  if(argc < 2)
  {
    fprintf(stderr, "Input an address.\n");
    return 1;
  }
  else if(strlen(argv[1]) > 15)
  {
    fprintf(stderr, "Address too long.\n");
    return 1;
  }
  
  sockopt.sin_family = AF_INET;
  sockopt.sin_port = htons(80);
  inet_aton(argv[1], &sockopt.sin_addr);

  if(getkey())
  {
    fprintf(stderr, "Failed to set up the terminal.\n");
    return 1;
  }

  while(1)
  {
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
      fprintf(stderr, "Can't open socket.\r\n");
      return 1;
    }
    else if(connect(sockfd, (struct sockaddr *) &sockopt, sizeof sockopt) < 0)
    {
      fprintf(stderr, "Failed to connect.\r\n");
      return 1;
    }

    pad_connect();
  }
}

int pad_connect(void)
{
  int input_len;
  char *pad_input;

  switch(getchar())
  {
    case 'z':
      pad_setup(PAD_CROSS);
      break;
    case 'a':
      pad_setup(PAD_SQUARE);
      break;
    case 's':
      pad_setup(PAD_CIRCLE);
      break;
    case 'w':
      pad_setup(PAD_TRIANGLE);
      break;
    case 'q':
      pad_setup(PAD_L1);
      break;
    case 'e':
      pad_setup(PAD_R1);
      break;
    case 'x':
      pad_setup(PAD_L2);
      break;
    case 'c':
      pad_setup(PAD_R2);
      break;
    case '1':
      pad_setup(PAD_START);
      break;
    case '2':
      pad_setup(PAD_SELECT);
      break;
    case 'h':
      pad_setup(PAD_HOME);
      break;
    case 'H':
      pad_setup(PAD_HOME_HOLD);
      break;
    case 27: /* For arrow keys. */
      if(getchar() == '[')
        switch(getchar())
        {
          case 'A':
            pad_setup(PAD_UP);
            break;
          case 'B':
            pad_setup(PAD_DOWN);
            break;
          case 'C':
            pad_setup(PAD_RIGHT);
            break;
          case 'D':
            pad_setup(PAD_LEFT);
            break;
          default:
            close(sockfd);
	    return 1;
        }
      else
      {
        close(sockfd);
        return 1;
      }
      break;
    case 3: /* For Ctrl+C. */
      printf("Exiting...\r\n");
      exit(0);
    default:
      close(sockfd);
      return 1;
  }

  printf("%s\r\n", pad_input);
  write(sockfd, pad_input, input_len);
  free(pad_input);
  close(sockfd);

  return 0;
}

int getkey(void)
{
  struct termios term;

  if(tcgetattr(0, &term) < 0) return 1;
  cfmakeraw(&term);
  if(tcsetattr(0, TCSANOW, &term) < 0) return 1;

  return 0;
}
