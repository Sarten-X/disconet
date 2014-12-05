/************************************\
*   Copyright (C) 2007 by Sarten-X   *
\************************************/

#include <ncurses.h>
#include <cstdlib>
#include <ctime>
#include <unistd.h>

#include <iostream>
#include <sstream>
#include <string>
#include <math.h>

#ifdef linux
#include <fstream>
#else
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <net/if.h>
#include <net/if_dl.h>
#include <net/route.h>
#include <sys/param.h>
#include <sys/sysctl.h>

unsigned long long* get_BSD_stats(unsigned long long* buffer, const char* interface);
#endif

using namespace std;

const int MIN_HEIGHT = 2;
const int MIN_WIDTH = 2;

// Function for creating an open box.
void create_box(int y, int x, int w, int h)
{
  mvaddch(y, x, ' ');
  mvaddch(y, x + w, ' ');
  mvaddch(y + h, x, ' ');
  mvaddch(y + h, x + w, ' ');
  mvhline(y, x + 1, ' ', w - 1);
  mvhline(y + h, x + 1, ' ', w - 1);
  mvvline(y + 1, x, ' ', h - 1);
  mvvline(y + 1, x + w, ' ', h - 1);
}

// Function for creating a filled box.
void fill_rect(int y, int x, int w, int h)
{
  mvhline(y, x, ' ', w);
  for (int n = 1; n < h; n++)
    mvhline(y+n, x, ' ', w);
}

struct net_state {
  size_t rcvbytes;
  size_t rcvpackets;
  size_t rcverrs;
  size_t rcvdrop;
  size_t rcvfifo;
  size_t rcvframe;
  size_t rcvcompressed;
  size_t rcvmulticast;

  size_t xmtbytes;
  size_t xmtpackets;
  size_t xmterrs;
  size_t xmtdrop;
  size_t xmtfifo;
  size_t xmtcolls;
  size_t xmtcarrier;
  size_t xmtcompressed;
};

int main(int argc, char* argv[])
{
  // Declare variables for the position of the tiles.
  int startx, starty, height, width, color, h, w;

  // Declare space for the number of tiles and storing old traffic calculation.
  long long number = -1;
  unsigned long long oldtraffic = 0;

  // Declare placeholders for all the fields in /proc/net/dev. I wallow in wasted RAM.
  net_state current, old;

  // Declare space and defaults for the scaling multiplier parameters.
  double xmultiplier = 1,ymultiplier = 1;

  // Declare space for the interface parameter.
  std::string chosen_interface;

#ifdef linux
  // Declare a file stream, for reading /proc/net/dev.
  fstream filestr;
#endif

  // Begin the main program

  // If a dumb number of parameters was supplied, print the usage message and exit.
  if(!(argc == 2 || argc == 4)) {
    fprintf(stderr, "Usage: %s <interface> [xmultiplier ymultiplier]\n", argv[0]);
    exit(1);
  } else {
    // Otherwise, remember the chosen interface.
    chosen_interface = argv[1];
    if (argc == 4) {
      // If scaling parameters were supplied, remember the scale given.
      xmultiplier = atof(argv[2]);
      ymultiplier = atof(argv[3]);
    }
  }

  // Start the ncurses control of the screen. Since only one window is used, it will take the whole screen.
  initscr();

  // Seed the random number generator with the current time, to achieve a good degree of randomness.
  srand((unsigned)time(0));

  // Configure the screen. I'm not sure what all these do.
  start_color();
  cbreak();
  keypad(stdscr, TRUE);
  noecho();

  // Set up color pairs, for use when drawing ncurses text.
  init_pair(0, COLOR_WHITE, COLOR_WHITE);
  init_pair(1, COLOR_RED, COLOR_RED);
  init_pair(2, COLOR_YELLOW, COLOR_YELLOW);
  init_pair(3, COLOR_GREEN, COLOR_GREEN);
  init_pair(4, COLOR_CYAN, COLOR_CYAN);
  init_pair(5, COLOR_BLUE, COLOR_BLUE);
  init_pair(6, COLOR_MAGENTA, COLOR_MAGENTA);
  init_pair(7, COLOR_BLACK, COLOR_BLACK);
  int colors = 8;

  // All configuration is done.

  /*

  	***** SAMPLE /proc/net/dev FILE FROM MY SERVER *****

  Inter-|   Receive                                                |  Transmit
   face |bytes    packets errs drop fifo frame compressed multicast|bytes    packets errs drop fifo colls carrier compressed
      lo:  989831   18734    0    0    0     0          0         0   989831   18734    0    0    0     0       0          0
    eth0:       0       0    0    0    0     0          0         0        0       0    0    0    0     0       0          0
    eth1:11933421   88557    0    0    0     0          0         0 43125253  104076    0    0    0     0       0          0


  */

  // Start main loop. This should have some kind of exit.
  while (1) {
    // Save a copy of old data, for calculating change.
    old = current;

#ifdef linux
    // Open the file /proc/net/dev for input.
    filestr.open ("/proc/net/dev", fstream::in);
    std::string header;
    // First two lines are table headers, skip those
    getline(filestr, header);
    getline(filestr, header);

    // Loop through each line of the file until we find the correct interface.
    while(filestr) {
      std::string interface;
      getline(filestr, interface, ':');
      if(interface == chosen_interface) break;
      else getline(filestr, interface);   // skip rest of the line
    }

    // Couldn't find the interface, exit "gracefully"
    if(!filestr) {
      endwin();
      std::cerr << "Failed to find interface" << std::endl;
      return -1;
    }

    // Load the new data from the stream into the variables
    filestr >>current.rcvbytes>>current.rcvpackets>>current.rcverrs>>current.rcvdrop>>current.rcvfifo>>current.rcvframe>>current.rcvcompressed>>current.rcvmulticast;
    filestr >>current.xmtbytes>>current.xmtpackets>>current.xmterrs>>current.xmtdrop>>current.xmtfifo>>current.xmtcolls>>current.xmtcarrier>>current.xmtcompressed;

    // Having found the interface, close the file stream.
    filestr.close();

#else
    {

      unsigned long long * stats = new unsigned long long[4];
      get_BSD_stats(stats, chosen_interface);
      current.rcvbytes = stats[0];
      current.rcvpackets = stats[1];
      current.xmtbytes = stats[2];
      current.xmtpackets = stats[3];
      delete[] stats;
    }
#endif

    // Begin calculations

    // Calculate number of tiles from data
    unsigned long long traffic = (current.xmtbytes + current.rcvbytes) / (1024);

    // If this is the first cycle (denoted by "number" being -1), set "number" to 0.
    // This will make the draw cycle not execute, but it will save the data for change calculations.
    if (number == -1) {
      number = 0;
    } else {
      number = traffic - oldtraffic;
    }

    oldtraffic = traffic;
    //cout << number << "	" << xmtbytes << "	" << rcvbytes << "	" << traffic << endl;
    // Calculate sizes
    //std::cout << rcvbytes << "	" << rcvpackets << "	" << xmtbytes << "	" << xmtpackets << "	" << std::endl;
    if (number > 0 && current.xmtpackets - old.xmtpackets > 0 && current.rcvpackets - old.rcvpackets > 0) {
      w = (current.xmtbytes-old.xmtbytes)/(current.xmtpackets - old.xmtpackets) ;	// Sent bytes per packet
      h = (current.rcvbytes-old.rcvbytes)/(current.rcvpackets - old.rcvpackets) ;	// Received bytes per packet
    } else {
      w = 0;
      h = 0;
    }

    for (long long i = 0; i< number; i++) {
      color = rand() % colors;

      //height = rand() % LINES/2;
      //width = rand() % COLS/2;
      //starty = (LINES - height)/2;
      //startx = (COLS - width)/2;

      starty = rand() % LINES;
      startx = rand() % COLS;

      //height = MIN_HEIGHT;
      //width = MIN_WIDTH;

      height = 1 * max(MIN_HEIGHT,(int)round(ymultiplier * MIN_HEIGHT/min(MIN_HEIGHT,MIN_WIDTH)*h/256));
      width = 2 * max(MIN_WIDTH, (int)round(xmultiplier * MIN_WIDTH/min(MIN_HEIGHT,MIN_WIDTH)*w/256));

      attron(COLOR_PAIR(color));


      //create_box(starty, startx, width, height));
      fill_rect(starty, startx, width, height);
    }

    //mvprintw(0,0,"");	// Move caret to row 0, column 0

    //mvprintw(1,0," %d %d %d %d %d %d %d %d ",rcvbytes,rcvpackets,rcverrs,rcvdrop,rcvfifo,rcvframe,rcvcompressed,rcvmulticast);
    //mvprintw(2,0," %d %d %d %d %d %d %d %d ", xmtbytes,xmtpackets,xmterrs,xmtdrop,xmtfifo,xmtcolls,xmtcarrier,xmtcompressed);

    usleep(100000);	// Wait for next interval
    refresh();
  }

  getch();

  endwin();
  return 0;
}

#ifndef linux
unsigned long long* get_BSD_stats(unsigned long long* out, const char* interface)
{
  // Based on someone else's code. I have no idea how it works.
  int mib[6] = { CTL_NET, PF_ROUTE, 0, 0, NET_RT_IFLIST, 0 };
  size_t len;
  char *buf, *next, *lim;
  struct if_data *ifd;
  struct rt_msghdr *rtm;
  struct sockaddr_dl *sdl;

  /* this magically tells us how much space we need */
  if( -1 == sysctl(mib, 6, NULL, &len, NULL, 0) ) {
    perror("sysctl");
    exit(1);
  }

  buf = (char *)malloc(len);
  if( NULL == buf ) {
    perror("malloc");
    exit(1);
  }

  if( -1 == sysctl(mib, 6, buf, &len, NULL, 0) ) {
    perror("sysctl");
    exit(1);
  }

  lim = buf + len;
  for(next = buf; next < lim; next += rtm->rtm_msglen) {
    rtm = (struct rt_msghdr *)next;
    if( RTM_VERSION == rtm->rtm_version &&
        RTM_IFINFO == rtm->rtm_type) {
      ifd = &((struct if_msghdr *)next)->ifm_data;

      /* dont ask, dont tell */
      sdl = (struct sockaddr_dl *)
            (next + sizeof(struct rt_msghdr) + 2 * sizeof(struct sockaddr));

      if( NULL == sdl || AF_LINK != sdl->sdl_family ) {
        continue;
      }
      if( 0 != strncmp(sdl->sdl_data, interface, sdl->sdl_nlen) ) {
        continue;
      }

      /* i don't think sdl->sdl_data is guaranteed to be null terminated */
      /*printf("%s %10lu %10lu %10lu %10lu\n", sdl->sdl_data,
      	ifd->ifi_ibytes, ifd->ifi_ipackets,
      	ifd->ifi_obytes, ifd->ifi_opackets);*/
      out[0] = ifd->ifi_ibytes;
      out[1] = ifd->ifi_ipackets;
      out[2] = ifd->ifi_obytes;
      out[3] = ifd->ifi_opackets;
    }
  }

  free(buf);
  return out;
}
#endif
