/************************************\
*   Copyright (C) 2007 by Sarten-X   *
\************************************/

#include "disconet.h"

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



int main(int argc, char* argv[])
{
  // Declare space and defaults for the scaling multiplier parameters.
  double xmultiplier = 1,ymultiplier = 1;

  // Declare space for the interface parameter.
  std::string chosen_interface;

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

  std::cout << "Finding network interface" << std::endl;
  // Attempt to get network statistics
  net_state interface_test = get_network_state(chosen_interface);

  // Couldn't find the interface, exit "gracefully"
  if (interface_test.error) {
    endwin();
    std::cerr << "Failed to find interface \"" << chosen_interface << "\"" << std::endl;
    return -1;
  }

  std::cout << "Initializing ncurses" << std::endl;
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

  // All configuration is done.
  std::cout << "Starting monitoring" << std::endl;
  loop(chosen_interface, xmultiplier, ymultiplier);
  getch();

  endwin();
  return 0;
}

void loop (std::string chosen_interface, double xmultiplier, double ymultiplier) {
  int colors = 8;
  // Declare variables for the position of the tiles.
  int startx, starty, height, width, color, h, w;

  // Declare space for the number of tiles and storing old traffic calculation.
  long long number = -1;
  unsigned long long oldtraffic = 0;

  // Declare placeholders for all the fields in /proc/net/dev. I wallow in wasted RAM.
  net_state current, old;

  // Start main loop. This should have some kind of exit.
  while (1) {
    // Save a copy of old data, for calculating change.
    old = current;
    current = get_network_state(chosen_interface);

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

      height = 1 * std::max(MIN_HEIGHT,(int)round(ymultiplier * MIN_HEIGHT/std::min(MIN_HEIGHT,MIN_WIDTH)*h/256));
      width = 2 * std::max(MIN_WIDTH, (int)round(xmultiplier * MIN_WIDTH/std::min(MIN_HEIGHT,MIN_WIDTH)*w/256));

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
}
