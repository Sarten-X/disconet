/************************************\
*   Copyright (C) 2007 by Sarten-X   *
\************************************/

#include "disconet.h"
#include "drawing.h"
#include <unistd.h>

#include <iostream>
#include <sstream>

void loop (std::string chosen_interface, double xscale, double yscale);

int main(int argc, char* argv[])
{
  // Declare space and defaults for the scaling scale parameters.
  double xscale = 1,yscale = 1;

  // Declare space for the interface parameter.
  std::string chosen_interface;

  // If a dumb number of parameters was supplied, print the usage message and exit.
  if(!(argc == 2 || argc == 4)) {
    std::cerr << "Usage: " << argv[0] << " <interface> [xscale yscale]" << std::endl;
    return 1;
  } 
  // Otherwise, remember the chosen interface.
  chosen_interface = argv[1];
  if (argc == 4) {
    // If scaling parameters were supplied, remember the scale given.
    xscale = atof(argv[2]);
    yscale = atof(argv[3]);
  }

  std::cout << "Finding network interface" << std::endl;
  // Attempt to get network statistics
  if (get_network_state(chosen_interface, NULL) != 0) {
    // Couldn't find the interface, exit "gracefully"
    std::cerr << "Failed to find interface \"" << chosen_interface << "\"" << std::endl;
    return -1;
  }

  std::cout << "Initializing ncurses" << std::endl;

  // All configuration is done.
  std::cout << "Starting monitoring" << std::endl;
  loop(chosen_interface, xscale, yscale);
  return 0;
}

void loop (std::string chosen_interface, double xscale, double yscale) {
  int h, w;

  // Declare space for the number of tiles and storing old traffic calculation.
  long long number = -1;
  unsigned long long oldtraffic = 0;

  // Declare placeholders for all the fields in /proc/net/dev. I wallow in wasted RAM.
  net_state current, old;

  initialize_drawing();

  // Start main loop. This should have some kind of exit.
  while (1) {
    // Save a copy of old data, for calculating change.
    old = current;
    if(get_network_state(chosen_interface, &current) != 0)
      break;

    // Begin calculations

    paint_drawing(current, xscale, yscale);

    usleep(100000);	// Wait for next interval
    refresh_drawing();
  }
  uninitialize_drawing();
}
