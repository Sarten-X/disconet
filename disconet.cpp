/************************************\
*   Copyright (C) 2007 by Sarten-X   *
\************************************/
#include "config.h"   // This is generated in build/config.h from <src>/config.h.in by cmake
#include "disconet.h"
#include "Output.h"
#include "Source.h"

#include <iostream>
#include <sstream>
#include <algorithm>    // std::random_shuffle

#include <sys/time.h>
#include <unistd.h>

INIT_PLUGIN_BUILDER

void loop (runtime_options options, Source* src, Output* output);

int exit_status = 0;

int main(int argc, char* argv[])
{
  runtime_options options;
  // Otherwise, remember the chosen interface.
  options.interface = "";
  options.profile = false;
  options.xscale = 1;
  options.yscale = 1;

  int option;
  Source* src = NULL;

  opterr = 0;
  while ((option = getopt (argc, argv, "h:w:i:p")) != -1) switch (option) {
    case 'h':
      options.yscale = atof(optarg);
      break;
    case 'w':
      options.xscale = atof(optarg);
      break;
    case 'i':
      options.interface = optarg;
      break;
    case 'p':
      options.profile = true;
      break;
    default:
      std::cerr << "Usage: " << argv[0] << "-i <interface> [-s source] [-w xscale] [-h yscale] [-p]" << std::endl;
      exit(1);
  }

  if (options.interface == "") {
    std::cerr << "Error: The -i option is required" << std::endl;
    exit(1);
  }

  if (options.profile) {
    std::vector<std::string> kinds = Source::kinds();
    if(std::find(kinds.begin(), kinds.end(), "PcapSource") != kinds.end())
      src = Source::create("PcapSource");
    else {
      std::cerr << "pcap source not available" << std::endl;
      return -1;
    }
  }
  if (src == NULL)
    src = Source::create("DefaultSource");

  std::cout << "Finding network interface" << std::endl;
  // Attempt to get network statistics
  if (src->get_network_state(options.interface, NULL) != 0) {
    // Couldn't find the interface, exit "gracefully"
    std::cerr << "Failed to find interface \"" << options.interface << "\"" << std::endl;
    return -1;
  }

  std::cout << "Initializing output" << std::endl;
  Output* output = Output::create("CursesOutput");
  output->initialize_drawing();
  // All configuration is done.
  std::cout << "Starting monitoring" << std::endl;
  loop(options, src, output);

  std::cout << "Cleaning up" << std::endl;
  output->uninitialize_drawing();
  return exit_status;
}

void loop (runtime_options options, Source* src, Output* output)
{
  timeval start_time, end_time;

  net_state current;

  // Start main loop. This should have some kind of exit.
  while (!exit_status) {
    gettimeofday(&start_time, NULL);


    std::map<dataType_t, net_state> states;
    if(src->get_network_state(options.interface, &states) != 0)
      break;

    // This belongs in the source routine. Sources should return this map.
    //states[current.type] = current;

    // Expand the map of network data into a vector of representative objects.
    // Each object represents a "typical" packet of a specific data type, scaled
    // according to the runtime options.
    std::vector<net_state> packets;
    for(std::map<dataType_t, net_state>::iterator iter=states.begin(); iter!=states.end(); ++iter) {
      current = iter->second;
      size_t object_count = (current.rcvbytes + current.xmtbytes) / BYTES_PER_OBJECT;
      size_t rcv_bytes_per_object = current.rcvbytes;// std::max(current.rcvpackets+current.xmtpackets,(long unsigned int)1);
      size_t xmt_bytes_per_object = current.xmtbytes; // std::max(current.rcvpackets+current.xmtpackets,(long unsigned int)1);
      for (int i = object_count; i > 0; --i) {
        net_state object;
        object.rcvbytes = rcv_bytes_per_object;
        object.xmtbytes = xmt_bytes_per_object;
        object.rcvpackets = 1;
        object.xmtpackets = 1;
        object.type = current.type;
        packets.push_back(object);
      }
    }

    std::random_shuffle ( packets.begin(), packets.end() );

    // Begin calculations
    //std::cout << current.rcvbytes << " " << current.rcvpackets << " " << current.xmtbytes << " " << current.xmtpackets << "\r" << std::endl;
    output->paint_drawing(packets, states, options.xscale, options.yscale);

    output->refresh_drawing();

    gettimeofday(&end_time, NULL);
    long int usec = REFRESH_TIME - (((end_time.tv_sec - start_time.tv_sec) * 1000000 + end_time.tv_usec) - start_time.tv_usec);
    if (usec > 0) usleep(usec);	// Wait for next interval
  }
}

int terminate(int reason) {
  exit_status = reason;
  return 0;
}
