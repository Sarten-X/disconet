#pragma once
#ifndef __DISONETDRAWING_H__
#define __DISONETDRAWING_H__

#include "Output.h"
class CursesOutput : public Output{
  public:
    CursesOutput();
    virtual ~CursesOutput();

    void initialize_drawing();
    void uninitialize_drawing();
    void paint_drawing(const std::vector<net_state> objects, const std::map<dataType_t, net_state> aggregate, double xmultiplier, double ymultiplier);
    void refresh_drawing();
  private:
    void create_box(int y, int x, int w, int h);
    void fill_rect(int y, int x, int w, int h);
    void draw_block(int w, int h, int type);
};

#endif // __DISONETDRAWING_H__
