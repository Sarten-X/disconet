#include "disconet.h"
#include "config.h"

#ifndef HAVE_PCAP
#error "Should not be compiling this, as pcap was not found."
#endif // HAVE_PCAP

#include <string>
#include <iostream>
#include <stdexcept>
#include <memory>

#include <pcap.h>

// Used for determining time to capture
#include <sys/time.h>
#include <unistd.h>

//#include <unordered_map>
#include <netinet/in.h>   // ntoh*
#include <net/ethernet.h> // struct ether_header
#include <netinet/tcp.h>  // struct tcphdr
#include <netinet/udp.h>  // struct udphdr
#include <netinet/ip.h>   // struct ip

class PCAPHandler {
public:
  PCAPHandler(const std::string& dev, const std::string& filter = std::string("")) {
    char errbuf[PCAP_ERRBUF_SIZE];
    inhandle =  pcap_open_live(dev.c_str(), BUFSIZ, 1, 1000, errbuf);
    if(!inhandle)
      throw std::runtime_error(errbuf);
    outhandle = pcap_open_live(dev.c_str(), BUFSIZ, 1, 1000, errbuf);
    if(!outhandle)
      throw std::runtime_error(errbuf);
    if (pcap_setdirection(inhandle,  PCAP_D_IN)  == -1)
      throw std::runtime_error("Can't set direction on rcv handle");
    if (pcap_setdirection(outhandle, PCAP_D_OUT) == -1)
      throw std::runtime_error("Can't set direction on xmt handle");
    setFilter(inhandle, filter, in_fp);
    setFilter(outhandle, filter, out_fp);
  }
  ~PCAPHandler() {
    if(inhandle) pcap_close(inhandle);
    if(outhandle) pcap_close(outhandle);
    pcap_freecode(&in_fp);
    pcap_freecode(&out_fp);
  }
  int get_network_state(net_state* state) {
    // TODO: replace "state" with "this" so we can encapsulate a std::map of a
    // bunch of states, segregating data with context (maybe by port numbers)
    pcap_dispatch(inhandle, -1, PCAPHandler::got_packet_rcv, (u_char*)state);
    pcap_dispatch(outhandle, -1, PCAPHandler::got_packet_xmt, (u_char*)state);
    return 0;
  }

private:
  void setFilter(pcap_t* hdl, const std::string& filter, struct bpf_program& fp) {
    if(pcap_compile(hdl, &fp, filter.c_str(), 0, PCAP_NETMASK_UNKNOWN) == -1)
      throw std::runtime_error(pcap_geterr(hdl));
    if(pcap_setfilter(hdl, &fp) == -1)
      throw std::runtime_error(pcap_geterr(hdl));
  }

  static void got_packet_rcv(u_char* user, const struct pcap_pkthdr* h, const u_char* bytes) {
    // TODO: Seperate this out, handle specific transports, etc in different functions.
    net_state* state = (net_state*)user;
    const uint8_t* data = bytes;
    const struct ether_header* ether = (const struct ether_header*)data;
    const struct ip* ip_ptr = (const struct ip*)(data += sizeof(struct ether_header));
    // Use caplen, as that is the size of the allocated packet.  len is what it
    // would have been had we captured everything (result of timing out, etc)
    if((bytes + h->caplen) < data) return;  // Didn't get the entire packet
    data += ip_ptr->ip_hl * 4;
    if((bytes + h->caplen) < data) return;  // Didn't get the entire packet
    switch(ip_ptr->ip_p) {
    case IPPROTO_TCP:
    {
      const struct tcphdr* tcp = (const struct tcphdr*)(data);
      data += tcp->doff * 4;
      if((bytes + h->caplen) < data) return;  // Didn't get the entire packet
      uint32_t len = h->caplen - (data - bytes);
      // Now we have the entire stack for a TCP packet, up to and including the payload
      switch(ntohs(tcp->th_dport)) {  // Remember endianess, use ntoh[sl]
      case 22:
        state->type = net_state::DATA_SSH; break;
      case 80:
      case 443:
        state->type = net_state::DATA_HTTP; break;
      // TODO: Add more application protocols here.
      default: break;
      }
    } break;
    case IPPROTO_UDP:
    {
      const struct udphdr* udp = (const struct udphdr*)(data);
      data += sizeof(struct udphdr);
      if((bytes + h->caplen) < data) return;  // Didn't get the entire packet
      uint32_t len = h->caplen - (data - bytes);
      // Now we have the entire stack for a UDP packet, up to and including the payload
    } break;
    // TODO: Other transport protocols here (defined in netinet/in.h)
    default: return;
    }

    ++state->rcvpackets;
    state->rcvbytes += h->len;
  }

  static void got_packet_xmt(u_char* user, const struct pcap_pkthdr* h, const u_char* bytes) {
    net_state* state = (net_state*)user;
    const uint8_t* data = bytes;
    const struct ether_header* ether = (const struct ether_header*)data;
    const struct ip* ip_ptr = (const struct ip*)(data += sizeof(struct ether_header));
    // Use caplen, as that is the size of the allocated packet.  len is what it
    // would have been had we captured everything (result of timing out, etc)
    if((bytes + h->caplen) < data) return;  // Didn't get the entire packet
    data += ip_ptr->ip_hl * 4;
    if((bytes + h->caplen) < data) return;  // Didn't get the entire packet
    switch(ip_ptr->ip_p) {
    case IPPROTO_TCP:
    {
      const struct tcphdr* tcp = (const struct tcphdr*)(data);
      data += tcp->doff * 4;
      if((bytes + h->caplen) < data) return;  // Didn't get the entire packet
      uint32_t len = h->caplen - (data - bytes);
      // Now we have the entire stack for a TCP packet, up to and including the payload
      switch(ntohs(tcp->th_dport)) {  // Remember endianess, use ntoh[sl]
      case 22:
        state->type = net_state::DATA_SSH; break;
      case 80:
      case 443:
        state->type = net_state::DATA_HTTP; break;
      // TODO: Add more application protocols here.
      default: break;
      }
    } break;
    case IPPROTO_UDP:
    {
      const struct udphdr* udp = (const struct udphdr*)(data);
      data += sizeof(struct udphdr);
      if((bytes + h->caplen) < data) return;  // Didn't get the entire packet
      uint32_t len = h->caplen - (data - bytes);
      // Now we have the entire stack for a UDP packet, up to and including the payload
    } break;
    // TODO: Other transport protocols here (defined in netinet/in.h)
    default: return;
    }

    ++state->xmtpackets;
    state->xmtbytes += h->len;
  }

  pcap_t* inhandle;
  pcap_t* outhandle;
  struct bpf_program in_fp, out_fp; // Not sure you need two fps, but they seem
                                    // to be per-handle.
};

static std::auto_ptr<PCAPHandler> handler;

int get_pcap_network_state(net_state* state) {
  if(handler.get() == NULL) return -1;
  handler->get_network_state(state);
  return 0;
}

int initialize_pcap(const std::string& interface) {
  try {
    handler.reset(new PCAPHandler(interface));
  } catch(const std::runtime_error& e) {
    std::cerr << "Failed to initialize pcap " << e.what() << std::endl;
    return -1;
  }
  return 0;
}
