#ifndef OUTPUT_H
#define OUTPUT_H

#include "disconet.h"
#include "plugin_builder.h"

#include <map>
#include <vector>

class Output
{
  public:
    Output();
    virtual ~Output();

    virtual void initialize_drawing() = 0;
    virtual void uninitialize_drawing() = 0;
    virtual void paint_drawing(const std::vector<net_state> objects, const std::map<dataType_t, net_state> aggregate, double xmultiplier, double ymultiplier) = 0;
    virtual void refresh_drawing() = 0;

    static std::vector<std::string> kinds() {
      std::vector<std::string> ret;
      typedef PluginBuilder<Output> PB;
      for(PB::FactoryMap::const_iterator it = PB::factoryMap().begin(); it != PB::factoryMap().end(); ++it)
        ret.push_back(it->first);
      return ret;
    }

    static Output* create(const std::string& kind) {
      return PluginBuilder<Output>::build(kind);
    }
};

#endif // OUTPUT_H
