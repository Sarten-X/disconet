#define DISCONET
#include <ncurses.h>
#include <cstdlib>
#include <ctime>
#include <unistd.h>

#include <iostream>
#include <sstream>
#include <string>
#include <math.h>


void loop(std::string chosen_interface, double xmultiplier, double ymultiplier);

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

  bool error;
};


#ifdef linux
#include "linux.h"
#else
#include "bsd.h"
#endif
