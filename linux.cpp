#include <string>
#include <fstream>
#include "disconet.h"

static void trim(std::string& s)
{
  size_t startpos = s.find_first_not_of(" \t"); //ltrim
  if( std::string::npos != startpos )
    s.erase(0, startpos);
}

/*
***** SAMPLE /proc/net/dev FILE FROM MY SERVER *****

Inter-|   Receive                                                |  Transmit
 face |bytes    packets errs drop fifo frame compressed multicast|bytes    packets errs drop fifo colls carrier compressed
    lo:  989831   18734    0    0    0     0          0         0   989831   18734    0    0    0     0       0          0
  eth0:       0       0    0    0    0     0          0         0        0       0    0    0    0     0       0          0
  eth1:11933421   88557    0    0    0     0          0         0 43125253  104076    0    0    0     0       0          0


*/

int get_network_state(const std::string& interface, net_state* state)
{

  std::string current_interface;
  // Declare a file stream, for reading /proc/net/dev.
  std::ifstream filestr("/proc/net/dev", std::fstream::in);
  if(!filestr) return -1;  // Couldn't open file

  {
    // First two lines are table headers, skip those
    std::string header;
    std::getline(filestr, header);
    std::getline(filestr, header);
  }

  // Loop through each line of the file until we find the correct interface.
  while(filestr) {
    std::string current_interface;
    std::getline(filestr, current_interface, ':');
    ::trim(current_interface);
    if(current_interface == interface)
      break;
    std::getline(filestr, current_interface);   // skip rest of the line
  }

  if(!filestr) return -1; // Couldn't find interface

  // Load the new data from the stream into the variables
  if(state != NULL)
    filestr >> state->rcvbytes >> state->rcvpackets >> state->rcverrs
            >> state->rcvdrop >> state->rcvfifo >> state->rcvframe
            >> state->rcvcompressed >> state->rcvmulticast
            >> state->xmtbytes >> state->xmtpackets >> state->xmterrs
            >> state->xmtdrop >> state->xmtfifo >> state->xmtcolls
            >> state->xmtcarrier >> state->xmtcompressed;

  return (!!filestr ? 0 : -1);
}
