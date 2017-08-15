#ifndef Laser_H
#define Laser_H

#include "ScreenComponents.h"

class Laser
{
  public:
    Screen screen;


    Laser();
    void draw();
    Frame getDigit(int x, int height = 50, int width = 20);
    static void setY(int y);
    static void setX(int x);
    static void drawPoint(Point p);
};

#endif