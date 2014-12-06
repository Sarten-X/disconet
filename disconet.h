#pragma once
#ifndef __DISCONET_H__
#define __DISCONET_H__

#include <cstdlib>
#include <string>
const int DATA_UNKNOWN = -1;

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

extern int get_network_state(const std::string& interface, net_state* state);

#endif // __DISCONET_H__
