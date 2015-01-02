#include <string>
#include <fstream>

#include "DefaultSource.h"
#include "plugin_builder.h"

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

int DefaultSource::get_network_state(const std::string& interface, StateMap* states)
{
  // dummy variables
  size_t rcverrs;
  size_t rcvdrop;
  size_t rcvfifo;
  size_t rcvframe;
  size_t rcvcompressed;
  size_t rcvmulticast;

  size_t xmterrs;
  size_t xmtdrop;
  size_t xmtfifo;
  size_t xmtcolls;
  size_t xmtcarrier;
  size_t xmtcompressed;

  old = current;
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
  filestr >> current.rcvbytes >> current.rcvpackets >> rcverrs
          >> rcvdrop >> rcvfifo >> rcvframe
          >> rcvcompressed >> rcvmulticast
          >> current.xmtbytes >> current.xmtpackets >> xmterrs
          >> xmtdrop >> xmtfifo >> xmtcolls
          >> xmtcarrier >> xmtcompressed;

  if (states != NULL) {
    net_state state;
    state.rcvbytes = current.rcvbytes - old.rcvbytes;
    state.xmtbytes = current.xmtbytes - old.xmtbytes;
    state.rcvpackets = current.rcvpackets - old.rcvpackets;
    state.xmtpackets = current.xmtpackets - old.xmtpackets;
    state.type = DATA_UNKNOWN;
    (*states)[state.type] = state;
  }

  return (!!filestr ? 0 : -1);
}

REGISTER_PLUGIN(Source, DefaultSource)
