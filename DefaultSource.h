#ifndef __DEFAULTSOURCE_H__
#define __DEFAULTSOURCE_H__

#include "Source.h"

class DefaultSource : public Source {
public:
  DefaultSource() {}
  ~DefaultSource() {}
  int get_network_state(const std::string& itfc, StateMap* states);
private:
#ifndef HAVE_IFDL_H
  net_state current;
  net_state old;
#endif
};

#endif // __DEFAULTSOURCE_H__
