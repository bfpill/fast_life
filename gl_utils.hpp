#ifndef GL_UTILS_HPP
#define GL_UTILS_HPP

#include <OpenGL/gl.h>
#include <GLUT/glut.h>
#include "game.hpp"

void set_pixel(int x, int y, float r, float g, float b);
void display_window();
void init_window(int argc, char** argv, Board board);
void set_global_board(const Board &board);

#endif // GL_UTILS_HPP

