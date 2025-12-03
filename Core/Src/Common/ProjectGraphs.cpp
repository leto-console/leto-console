#include <Common/ProjectGraphs.hpp>

#include <SSD1306/SSD1306_Properties.hpp>

//Graph<float, 128> Linked_Nx_Graph(SSD1306_Height, 1.0);
//Graph<float, 128> Linked_Ny_Graph(SSD1306_Height, 1.0);
//Graph<float, 128> Linked_Nz_Graph(SSD1306_Height, 1.0);
//
//Graph<float, 128> Linked_Wx_Graph(SSD1306_Height, 360.0);
//Graph<float, 128> Linked_Wy_Graph(SSD1306_Height, 360.0);
//Graph<float, 128> Linked_Wz_Graph(SSD1306_Height, 360.0);

Graph<char, 128> DEBUG_ENCODER_CLK(SSD1306_Height / 2, 1);
Graph<char, 128> DEBUG_ENCODER_DT(SSD1306_Height / 2, 1);
