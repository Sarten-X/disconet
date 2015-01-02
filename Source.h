#ifndef __SOURCE_H__
#define __SOURCE_H__

#include "disconet.h"
#include "plugin_builder.h"

class Source {
public:
  Source() {}
  virtual ~Source() {}
  virtual int get_network_state(const std::string& itfc, StateMap* states) = 0;

  static std::vector<std::string> kinds() {
    std::vector<std::string> ret;
    typedef PluginBuilder<Source> PB;
    for(PB::FactoryMap::const_iterator it = PB::factoryMap().begin(); it != PB::factoryMap().end(); ++it)
      ret.push_back(it->first);
    return ret;
  }

  static Source* create(const std::string& kind) {
    return PluginBuilder<Source>::build(kind);
  }
};

#endif // __SOURCE_H__
