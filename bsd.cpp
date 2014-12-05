#include "bsd.h"
#ifndef linux

net_state get_network_state(std::string interface) {
  net_state current;
  {

    unsigned long long * stats = new unsigned long long[4];
    get_BSD_stats(stats, chosen_interface);
    current.rcvbytes = stats[0];
    current.rcvpackets = stats[1];
    current.xmtbytes = stats[2];
    current.xmtpackets = stats[3];
    delete[] stats;
  }
  return current;
}

unsigned long long* get_BSD_stats(unsigned long long* out, const char* interface)
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
    exit(1);
  }

  buf = (char *)malloc(len);
  if( NULL == buf ) {
    perror("malloc");
    exit(1);
  }

  if( -1 == sysctl(mib, 6, buf, &len, NULL, 0) ) {
    perror("sysctl");
    exit(1);
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
      out[0] = ifd->ifi_ibytes;
      out[1] = ifd->ifi_ipackets;
      out[2] = ifd->ifi_obytes;
      out[3] = ifd->ifi_opackets;
    }
  }

  free(buf);
  return out;
}

#endif
