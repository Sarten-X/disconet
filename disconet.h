#pragma once
#ifndef __DISCONET_H__
#define __DISCONET_H__

#include <cstdlib>
#include <string>

#define DIRECTION_SEND (0)
#define BYTES_PER_OBJECT (2048)
#define REFRESH_TIME (100000)

#include <map>
#include <vector>

enum dataType_t {
  DATA_UNKNOWN = 0,
  DATA_HTTP = 1,
  DATA_SSH = 2,
  NDATA_TYPES = 3
};

struct net_state {
  size_t rcvbytes;
  size_t rcvpackets;
  size_t xmtbytes;
  size_t xmtpackets;

  dataType_t type;
};

struct runtime_options {
  double xscale;
  double yscale;
  bool profile;
  std::string interface;
};

int terminate(int reason);

extern int get_network_state(const std::string& interface, net_state* state);

extern int initialize_pcap(const std::string& interface);
extern int get_pcap_network_state(net_state* state);
#endif // __DISCONET_H__
