/*
 * ProjectGraphs.hpp
 *
 *  Created on: Nov 08, 2025
 *      Author: Timur
 */

#ifndef INC_COMMON_PROJECT_GRAPHS_HPP_
#define INC_COMMON_PROJECT_GRAPHS_HPP_

#include <Drawable/Graph.hpp>

//// График проекции на ось X кажущегося ускорения в базисе связной СК
//extern Graph<float, 128> Linked_Nx_Graph;
//// График проекции на ось Y кажущегося ускорения в базисе связной СК
//extern Graph<float, 128> Linked_Ny_Graph;
//// График проекции на ось Z кажущегося ускорения в базисе связной СК
//extern Graph<float, 128> Linked_Nz_Graph;
//
//// График проекции на ось X абсолютной угловой скорости связной СК
//extern Graph<float, 128> Linked_Wx_Graph;
//// График проекции на ось Y абсолютной угловой скорости связной СК
//extern Graph<float, 128> Linked_Wy_Graph;
//// График проекции на ось Z абсолютной угловой скорости связной СК
//extern Graph<float, 128> Linked_Wz_Graph;

// Для отладки
extern Graph<char, 128> DEBUG_ENCODER_CLK;
extern Graph<char, 128> DEBUG_ENCODER_DT;

#endif