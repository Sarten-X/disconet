
#include <pcap.h>
#ifdef PCAP_VERSION_MAJOR
#include <string>
#include <iostream>

// Used for determining time to capture
#include <sys/time.h>
#include <unistd.h>

//#include <unordered_map>
#include <netinet/in.h>
#include "disconet.h"

//std::unordered_map<int, net_state> traffic_profile;
net_state profile;
pcap_t *handle;			/* Session handle */
pcap_t *handle2;			/* Session handle */


////////////////////////////////////////////////////////////////////////////////
// From "Programming with pcap"
// http://www.tcpdump.org/pcap.html
//
// Copyright 2002 Tim Carstens. All rights reserved. Redistribution and use,
// with or without modification, are permitted provided that the following
// conditions are met:
//
// Redistribution must retain the above copyright notice and this list of
// conditions.
// The name of Tim Carstens may not be used to endorse or promote products
// derived from this document without specific prior written permission.
////////////////////////////////////////////////////////////////////////////////

/* Ethernet addresses are 6 bytes */
#define ETHER_ADDR_LEN	6

/* Ethernet header */
struct sniff_ethernet {
  u_char ether_dhost[ETHER_ADDR_LEN]; /* Destination host address */
  u_char ether_shost[ETHER_ADDR_LEN]; /* Source host address */
  u_short ether_type; /* IP? ARP? RARP? etc */
};

/* IP header */
struct sniff_ip {
  u_char ip_vhl;		/* version << 4 | header length >> 2 */
  u_char ip_tos;		/* type of service */
  u_short ip_len;		/* total length */
  u_short ip_id;		/* identification */
  u_short ip_off;		/* fragment offset field */
#define IP_RF 0x8000		/* reserved fragment flag */
#define IP_DF 0x4000		/* dont fragment flag */
#define IP_MF 0x2000		/* more fragments flag */
#define IP_OFFMASK 0x1fff	/* mask for fragmenting bits */
  u_char ip_ttl;		/* time to live */
  u_char ip_p;		/* protocol */
  u_short ip_sum;		/* checksum */
  struct in_addr ip_src,ip_dst; /* source and dest address */
};
#define IP_HL(ip)		(((ip)->ip_vhl) & 0x0f)
#define IP_V(ip)		(((ip)->ip_vhl) >> 4)

/* TCP header */
typedef u_int tcp_seq;

struct sniff_tcp {
  u_short th_sport;	/* source port */
  u_short th_dport;	/* destination port */
  tcp_seq th_seq;		/* sequence number */
  tcp_seq th_ack;		/* acknowledgement number */
  u_char th_offx2;	/* data offset, rsvd */
#define TH_OFF(th)	(((th)->th_offx2 & 0xf0) >> 4)
  u_char th_flags;
#define TH_FIN 0x01
#define TH_SYN 0x02
#define TH_RST 0x04
#define TH_PUSH 0x08
#define TH_ACK 0x10
#define TH_URG 0x20
#define TH_ECE 0x40
#define TH_CWR 0x80
#define TH_FLAGS (TH_FIN|TH_SYN|TH_RST|TH_ACK|TH_URG|TH_ECE|TH_CWR)
  u_short th_win;		/* window */
  u_short th_sum;		/* checksum */
  u_short th_urp;		/* urgent pointer */
};
/* ethernet headers are always exactly 14 bytes */
#define SIZE_ETHERNET 14
const struct sniff_ethernet *ethernet; /* The ethernet header */
const struct sniff_ip *ip; /* The IP header */
const struct sniff_tcp *tcp; /* The TCP header */

u_int size_ip;
u_int size_tcp;

net_state get_packet_data (const struct pcap_pkthdr *header, const u_char *packet) {
  net_state state;
  state.rcvbytes = 0;
  state.xmtbytes = 0;
  state.rcvpackets = 0;
  state.xmtpackets = 0;
  state.type = DATA_UNKNOWN;

  ethernet = (struct sniff_ethernet*)(packet);
	ip = (struct sniff_ip*)(packet + SIZE_ETHERNET);
	size_ip = IP_HL(ip)*4;

	if (size_ip < 20) {
		//printf("   * Invalid IP header length: %u bytes\n", size_ip);
		state.type = DATA_UNKNOWN;
	} else {
    tcp = (struct sniff_tcp*)(packet + SIZE_ETHERNET + size_ip);
    size_tcp = TH_OFF(tcp)*4;
    if (size_tcp < 20) {
      //printf("   * Invalid TCP header length: %u bytes\n", size_tcp);
      state.type = DATA_UNKNOWN;
    } else {

      if (tcp->th_dport == 22 || tcp->th_sport == 22) {
        state.type = DATA_SSH;
      }

      if (tcp->th_dport == 80 || tcp->th_sport == 80 ||
          tcp->th_dport == 443 || tcp->th_sport == 443) {
        state.type = DATA_HTTP;
      }

      state.rcvbytes += ip->ip_len;
      return state;
    }
  }
}
////////////////////////////////////////////////////////////////////////////////
// End derivative work
////////////////////////////////////////////////////////////////////////////////

void got_packet_rcv(u_char *args, const struct pcap_pkthdr *header, const u_char *packet) {
  //net_state single_state = get_packet_data(header, packet);
  //net_state profile = //traffic_profile[single_state.type];
  profile.rcvbytes += header->len;
  profile.rcvpackets++;
  //traffic_profile[single_state.type] = profile;
}
void got_packet_xmt(u_char *args, const struct pcap_pkthdr *header, const u_char *packet) {
  //net_state single_state = get_packet_data(header, packet);
  //net_state profile = traffic_profile[single_state.type];
  profile.xmtbytes += header->len;
  profile.xmtpackets++;
  //traffic_profile[single_state.type] = profile;
}

int get_pcap_network_state(net_state* state) {
  pcap_dispatch(handle, -1, got_packet_rcv, NULL);
  pcap_dispatch(handle2, -1, got_packet_xmt, NULL);

  state->rcvbytes = profile.rcvbytes;
  state->rcvpackets = profile.rcvpackets;
  state->xmtbytes = profile.xmtbytes;
  state->xmtpackets = profile.xmtpackets;

  //std::cout << "PCAP: " << profile.rcvbytes << " " << profile.rcvpackets << " " << profile.xmtbytes << " " << profile.xmtpackets << "\r" << std::endl;
  //std::cerr << "ERR: " << pcap_geterr(handle) << "        " << pcap_geterr(handle2) << std::endl;
  return 0;
}

int initialize_pcap(const std::string& interface) {
  const char *dev = interface.c_str();			/* The device to sniff on */
  char errbuf[PCAP_ERRBUF_SIZE];	/* Error string */
  struct bpf_program fp;		/* The compiled filter */
  char filter_exp[] = "";	/* The filter expression */
  bpf_u_int32 mask;		/* Our netmask */
  bpf_u_int32 net;		/* Our IP */
  //struct pcap_pkthdr header;	/* The header that pcap gives us */
  //const u_char *packet;		/* The actual packet */

  /* Find the properties for the device */
  if (pcap_lookupnet(dev, &net, &mask, errbuf) == -1) {
    std::cerr << "Couldn't get netmask for device " << dev << ": " << errbuf << std::endl;
    net = 0;
    mask = 0;
  }
  /* Open the session in promiscuous mode */
  handle = pcap_open_live(dev, BUFSIZ, 1, 1000, errbuf);
  if (handle == NULL) {
    std::cerr << "Couldn't open device " << dev << ": " << errbuf << std::endl;
    return(2);
  }
  /* Compile and apply the filter */
  if (pcap_compile(handle, &fp, filter_exp, 0, net) == -1) {
    std::cerr << "Couldn't parse filter " << filter_exp << ": " << pcap_geterr(handle) << std::endl;
    return(2);
  }
  if (pcap_setfilter(handle, &fp) == -1) {
    std::cerr << "Couldn't install filter " << filter_exp << ": " << pcap_geterr(handle) << std::endl;
    return(2);
  }
  if (pcap_setdirection(handle, PCAP_D_IN) == -1) {
    std::cerr << "Couldn't set direction IN: " << pcap_geterr(handle) << std::endl;
    return(2);
  }

  /* Open the session in promiscuous mode */
  handle2 = pcap_open_live(dev, BUFSIZ, 1, 1000, errbuf);
  if (handle == NULL) {
    std::cerr << "Couldn't open device " << dev << ": " << errbuf << std::endl;
    return(2);
  }
  /* Compile and apply the filter */
  if (pcap_compile(handle2, &fp, filter_exp, 0, net) == -1) {
    std::cerr << "Couldn't parse filter " << filter_exp << ": " << pcap_geterr(handle) << std::endl;
    return(2);
  }
  if (pcap_setfilter(handle2, &fp) == -1) {
    std::cerr << "Couldn't install filter " << filter_exp << ": " << pcap_geterr(handle) << std::endl;
    return(2);
  }
  if (pcap_setdirection(handle2, PCAP_D_OUT) == -1) {
    std::cerr << "Couldn't set direction OUT: " << pcap_geterr(handle) << std::endl;
    return(2);
  }

  return 0;
}

#endif // PCAP_VERSION_MAJOR
