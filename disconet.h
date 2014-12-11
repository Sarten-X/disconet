#pragma once
#ifndef __DISCONET_H__
#define __DISCONET_H__

#include <cstdlib>
#include <string>
const int DATA_UNKNOWN = 0;
const int DATA_HTTP = 1;
const int DATA_SSH = 2;

const int DIRECTION_SEND = 0;

const int REFRESH_TIME = 100000;

struct net_state {
  size_t rcvbytes = 0;
  size_t rcvpackets = 0;
  size_t rcverrs = 0;
  size_t rcvdrop = 0;
  size_t rcvfifo = 0;
  size_t rcvframe = 0;
  size_t rcvcompressed = 0;
  size_t rcvmulticast = 0;

  size_t xmtbytes = 0;
  size_t xmtpackets = 0;
  size_t xmterrs = 0;
  size_t xmtdrop = 0;
  size_t xmtfifo = 0;
  size_t xmtcolls = 0;
  size_t xmtcarrier = 0;
  size_t xmtcompressed = 0;

  int type = DATA_UNKNOWN;
};

extern int get_network_state(const std::string& interface, net_state* state);


extern int initialize_pcap(const std::string& interface);
extern int get_pcap_network_state(net_state* state);
#endif // __DISCONET_H__
