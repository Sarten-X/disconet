#ifndef __PCAPSOURCE_H__
#define __PCAPSOURCE_H__

#include "Source.h"

class PcapPimpl;

class PcapSource : public Source {
public:
  PcapSource();
  ~PcapSource();
  int get_network_state(const std::string& itfc, StateMap* states);
private:
  PcapPimpl* pimpl;
  std::string itfc;
};

#endif // __PCAPSOURCE_H__
