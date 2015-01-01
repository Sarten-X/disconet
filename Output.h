#ifndef OUTPUT_H
#define OUTPUT_H

#include "disconet.h"
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
  protected:
  private:
};

#endif // OUTPUT_H
