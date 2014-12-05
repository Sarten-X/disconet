#ifndef DISCONET
#include "disconet.h"
#endif // DISCONET
#ifdef linux

#include <fstream>

net_state get_network_state(std::string interface);

#endif
