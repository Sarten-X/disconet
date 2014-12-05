/************************************\
*   Copyright (C) 2007 by Sarten-X   *
\************************************/

#include "disconet.h"
#include <unistd.h>

#include <iostream>
#include <sstream>


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
    std::cerr << "Failed to find interface \"" << chosen_interface << "\"" << std::endl;
    return -1;
  }

  std::cout << "Initializing ncurses" << std::endl;
  initialize_drawing();

  // All configuration is done.
  std::cout << "Starting monitoring" << std::endl;
  loop(chosen_interface, xmultiplier, ymultiplier);
  uninitialize_drawing();
  return 0;
}

void loop (std::string chosen_interface, double xmultiplier, double ymultiplier) {
  int h, w;

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
      draw_block(xmultiplier,ymultiplier, w, h, DATA_UNKNOWN);
    }

    //mvprintw(0,0,"");	// Move caret to row 0, column 0

    //mvprintw(1,0," %d %d %d %d %d %d %d %d ",rcvbytes,rcvpackets,rcverrs,rcvdrop,rcvfifo,rcvframe,rcvcompressed,rcvmulticast);
    //mvprintw(2,0," %d %d %d %d %d %d %d %d ", xmtbytes,xmtpackets,xmterrs,xmtdrop,xmtfifo,xmtcolls,xmtcarrier,xmtcompressed);

    usleep(100000);	// Wait for next interval
    refresh_drawing();
  }
}
