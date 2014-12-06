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
#include "disconet.h"

int get_network_state(const std::string& interface, net_state* state) {
  return get_BSD_stats(state, interface.c_str());
}

static int get_BSD_stats(net_state* out, const char* interface)
{
  // Based on someone else's code. I have no idea how it works.
  int mib[6] = { CTL_NET, PF_ROUTE, 0, 0, NET_RT_IFLIST, 0 };
  size_t len;
  char *buf, *next, *lim;
  struct if_data *ifd;
  struct rt_msghdr *rtm;
  struct sockaddr_dl *sdl;

  /* this magically tells us how much space we need */
  if( -1 == sysctl(mib, 6, NULL, &len, NULL, 0) ) {
    perror("sysctl");
    return -1;
  }

  buf = (char *)malloc(len);
  if( NULL == buf ) {
    perror("malloc");
    return -1;
  }

  if( -1 == sysctl(mib, 6, buf, &len, NULL, 0) ) {
    perror("sysctl");
    return -1;
  }

  lim = buf + len;
  for(next = buf; next < lim; next += rtm->rtm_msglen) {
    rtm = (struct rt_msghdr *)next;
    if( RTM_VERSION == rtm->rtm_version &&
        RTM_IFINFO == rtm->rtm_type) {
      ifd = &((struct if_msghdr *)next)->ifm_data;

      /* dont ask, dont tell */
      sdl = (struct sockaddr_dl *)
            (next + sizeof(struct rt_msghdr) + 2 * sizeof(struct sockaddr));

      if( NULL == sdl || AF_LINK != sdl->sdl_family ) {
        continue;
      }
      if( 0 != strncmp(sdl->sdl_data, interface, sdl->sdl_nlen) ) {
        continue;
      }

      /* i don't think sdl->sdl_data is guaranteed to be null terminated */
      /*printf("%s %10lu %10lu %10lu %10lu\n", sdl->sdl_data,
      	ifd->ifi_ibytes, ifd->ifi_ipackets,
      	ifd->ifi_obytes, ifd->ifi_opackets);*/
      if(out != NULL) {
        out->rcvbytes = ifd->ifi_ibytes;
        out->rcvpackets = ifd->ifi_ipackets;
        out->xmtbytes = ifd->ifi_obytes;
        out->xmtpackets = ifd->ifi_opackets;
      }
      free(buf);
      return 0;
    }
  }
  free(buf);
  return -2;  // Couldn't find interface
}
