#pragma once
#ifndef __DISONETDRAWING_H__
#define __DISONETDRAWING_H__

void initialize_drawing();

void uninitialize_drawing();

void paint_drawing(const net_state& state, double xmultipler, double ymultipler);

void refresh_drawing();

#endif // __DISONETDRAWING_H__
