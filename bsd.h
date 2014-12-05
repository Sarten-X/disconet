#ifndef DISCONET
#include "disconet.h"
#endif // DISCONET

#ifndef linux
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <net/if.h>
#include <net/if_dl.h>
#include <net/route.h>
#include <sys/param.h>
#include <sys/sysctl.h>

unsigned long long* get_BSD_stats(unsigned long long* buffer, const char* interface);

net_state get_network_state(std::string interface);

#endif
