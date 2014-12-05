#include "linux.h"
#ifdef linux

net_state get_network_state(std::string interface) {
  net_state current;

  std::string current_interface;
  // Declare a file stream, for reading /proc/net/dev.
  std::fstream filestr;

  // Open the file /proc/net/dev for input.
    filestr.open ("/proc/net/dev", std::fstream::in);

  /*

  	***** SAMPLE /proc/net/dev FILE FROM MY SERVER *****

  Inter-|   Receive                                                |  Transmit
   face |bytes    packets errs drop fifo frame compressed multicast|bytes    packets errs drop fifo colls carrier compressed
      lo:  989831   18734    0    0    0     0          0         0   989831   18734    0    0    0     0       0          0
    eth0:       0       0    0    0    0     0          0         0        0       0    0    0    0     0       0          0
    eth1:11933421   88557    0    0    0     0          0         0 43125253  104076    0    0    0     0       0          0


  */
  std::string header;
  // First two lines are table headers, skip those
  getline(filestr, header);
  getline(filestr, header);

  // Loop through each line of the file until we find the correct interface.
  while(filestr) {
    std::string current_interface;
    getline(filestr, current_interface, ':');
    size_t startpos = current_interface.find_first_not_of(" \t");
    if( std::string::npos != startpos ) {
        current_interface = current_interface.substr( startpos );
    }
    if(current_interface == interface) {
      current.error = false;
      break;
    }
    else {
      current.error = true;
      getline(filestr, current_interface);   // skip rest of the line
    }
  }

  // Load the new data from the stream into the variables
  filestr >>current.rcvbytes>>current.rcvpackets>>current.rcverrs>>current.rcvdrop>>current.rcvfifo>>current.rcvframe>>current.rcvcompressed>>current.rcvmulticast;
  filestr >>current.xmtbytes>>current.xmtpackets>>current.xmterrs>>current.xmtdrop>>current.xmtfifo>>current.xmtcolls>>current.xmtcarrier>>current.xmtcompressed;

  // Having found the interface, close the file stream.
  filestr.close();

  return current;
}

#endif
