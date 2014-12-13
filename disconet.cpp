/************************************\
*   Copyright (C) 2007 by Sarten-X   *
\************************************/
#include "config.h"   // This is generated in build/config.h from <src>/config.h.in by cmake
#include "disconet.h"
#include "drawing.h"

#include <iostream>
#include <sstream>

#include <sys/time.h>
#include <unistd.h>

void loop (runtime_options options);

int main(int argc, char* argv[])
{
  runtime_options options;
  // Otherwise, remember the chosen interface.
  options.interface = "";
  options.profile = false;
  options.xscale = 1;
  options.yscale = 1;

  // TODO: Reimplement a help message
  // If a dumb number of parameters was supplied, print the usage message and exit.
  //if(!(argc == 2 || argc == 4)) {
  //  std::cerr << "Usage: " << argv[0] << " <interface> [xscale yscale]" << std::endl;
  //  return 1;
  //}
  int option;

  opterr = 0;
  while ((option = getopt (argc, argv, "h:w:i:p")) != -1) switch (option) {
    case 'h':
      options.yscale = atof(optarg);
    case 'w':
      options.xscale = atof(optarg);
    case 'i':
      options.interface = optarg;
    case 'p':
      options.profile = true;
  }

  if (options.interface == "") {
    options.interface = argv[optind];
  }


  std::cout << "Finding network interface" << std::endl;
  // Attempt to get network statistics
  if (get_network_state(options.interface, NULL) != 0) {
    // Couldn't find the interface, exit "gracefully"
    std::cerr << "Failed to find interface \"" << options.interface << "\"" << std::endl;
    return -1;
  }

  if (options.profile) {
    #ifdef HAVE_PCAP
    std::cout << "Initializing pcap" << std::endl;
    if (initialize_pcap(options.interface) != 0) {
      // Couldn't find the interface, exit "gracefully"
      std::cerr << "Failed to initialize pcap for \"" << options.interface << "\"" << std::endl;
      return -1;
    }
    #else
    std::cerr << "Not compiled with libpcap support. Profiling not available." << std::endl;
    return -1;
    #endif  // HAVE_PCAP
  }

  std::cout << "Initializing ncurses" << std::endl;
  initialize_drawing();
  // All configuration is done.
  std::cout << "Starting monitoring" << std::endl;
  loop(options);

  std::cout << "Cleaning up" << std::endl;
  uninitialize_drawing();
  return 0;
}

void loop (runtime_options options)
{
  int h, w;

  timeval start_time, end_time;

  // Declare space for the number of tiles and storing old traffic calculation.
  long long number = -1;
  unsigned long long oldtraffic = 0;

  // Declare placeholders for all the fields in /proc/net/dev. I wallow in wasted RAM.
  net_state current, old;

  // Start main loop. This should have some kind of exit.
  while (1) {
    gettimeofday(&start_time, NULL);
    // Save a copy of old data, for calculating change.
    old = current;

    if (options.profile) {
      #ifdef HAVE_PCAP
      if(get_pcap_network_state(&current) != 0)
        break;
      #endif // HAVE_PCAP
    } else {
      if(get_network_state(options.interface, &current) != 0)
        break;
    }

    // Begin calculations
    //std::cout << current.rcvbytes << " " << current.rcvpackets << " " << current.xmtbytes << " " << current.xmtpackets << "\r" << std::endl;
    paint_drawing(current, options.xscale, options.yscale);

    gettimeofday(&end_time, NULL);
    long int usec = REFRESH_TIME - (((end_time.tv_sec - start_time.tv_sec) * 1000000 + end_time.tv_usec) - start_time.tv_usec);
    if (usec > 0) usleep(usec);	// Wait for next interval
    refresh_drawing();
  }
}
