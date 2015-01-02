#include "PcapSource.h"
#include "config.h"
#include "plugin_builder.h"

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

#include <map>
#include <netinet/in.h>   // ntoh*
#include <net/ethernet.h> // struct ether_header
#include <netinet/tcp.h>  // struct tcphdr
#include <netinet/udp.h>  // struct udphdr
#include <netinet/ip.h>   // struct ip

class PcapPimpl
{
public:
  PcapPimpl(const std::string& dev, const std::string& filter = std::string(""))
  {
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

  ~PcapPimpl()
  {
    if(inhandle) pcap_close(inhandle);
    if(outhandle) pcap_close(outhandle);
    pcap_freecode(&in_fp);
    pcap_freecode(&out_fp);
  }

  int get_network_state(StateMap* return_states)
  {
    pcap_dispatch(inhandle, -1, PcapPimpl::got_packet_rcv, (u_char*)return_states);
    pcap_dispatch(outhandle, -1, PcapPimpl::got_packet_xmt, (u_char*)return_states);
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
    StateMap* states = (StateMap*)user;
    process_packet(states, h, bytes, PCAP_D_IN);
  }

  static void got_packet_xmt(u_char* user, const struct pcap_pkthdr* h, const u_char* bytes) {
    StateMap* states = (StateMap*)user;
    process_packet(states, h, bytes, PCAP_D_OUT);
  }

  static void process_packet (StateMap* states, const struct pcap_pkthdr* h, const u_char* bytes, int direction) {
    net_state current;
    current.type = DATA_UNKNOWN;
    const uint8_t* data = bytes;
    const struct ether_header* ether = (const struct ether_header*)data;
    const struct ip* ip_ptr = (const struct ip*)(data += sizeof(struct ether_header));
    // Use caplen, as that is the size of the allocated packet.  len is what it
    // would have been had we captured everything (result of timing out, etc)
    if((bytes + h->caplen) < data) return;  // Didn't get the entire packet
    data += ip_ptr->ip_hl * 4;
    if((bytes + h->caplen) < data) return;  // Didn't get the entire packet
    switch(ip_ptr->ip_p) {
      case IPPROTO_TCP: {
        const struct tcphdr* tcp = (const struct tcphdr*)(data);
        data += tcp->doff * 4;
        if((bytes + h->caplen) < data) return;  // Didn't get the entire packet
        uint32_t len = h->caplen - (data - bytes);
        // Now we have the entire stack for a TCP packet, up to and including the payload
        uint16_t target_address;
        if (direction == PCAP_D_IN) {
          target_address = tcp->dest;
        } else if (direction == PCAP_D_OUT) {
          target_address = tcp->source;
        }

        switch(ntohs(target_address)) {
          case 22:
            current.type = DATA_SSH;
            break;
          case 80:
          case 443:
            current.type = DATA_HTTP;
            break;
          // TODO: Add more application protocols here.
          default:
            break;
      }
    }
    break;
    case IPPROTO_UDP: {
      const struct udphdr* udp = (const struct udphdr*)(data);
      data += sizeof(struct udphdr);
      if((bytes + h->caplen) < data) return;  // Didn't get the entire packet
      uint32_t len = h->caplen - (data - bytes);
      // Now we have the entire stack for a UDP packet, up to and including the payload
    }
    break;
    // TODO: Other transport protocols here (defined in netinet/in.h)
    default:
      return;
    }

    net_state& old_state = (*states)[current.type];
    if (direction == PCAP_D_IN) {
      old_state.rcvbytes += h->len;
      ++old_state.xmtpackets;
    } else if (direction == PCAP_D_OUT) {
      old_state.xmtbytes += h->len;
      ++old_state.xmtpackets;
    }
  }

  pcap_t* inhandle;
  pcap_t* outhandle;
  struct bpf_program in_fp, out_fp; // Not sure you need two fps, but they seem
  // to be per-handle.
};

PcapSource::PcapSource() : pimpl(NULL) {}

PcapSource::~PcapSource() {
  if(pimpl) {
    delete pimpl;
    pimpl = NULL;
  }
}

int PcapSource::get_network_state(const std::string& itfc, StateMap* states)
{
  if(this->itfc != itfc)
  {
    if(pimpl) {
      delete pimpl;
      pimpl = NULL;
    }
    try {
      pimpl = new PcapPimpl(itfc);
    } catch(const std::runtime_error& e) {
      std::cerr << "Failed to initialize pcap " << e.what() << std::endl;
      return -1;
    }
  }
  pimpl->get_network_state(states);
  return 0;
}

REGISTER_PLUGIN(Source, PcapSource)
