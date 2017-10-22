#ifndef Laser_H
#define Laser_H

#include "ScreenComponents.h"


typedef std::array<int, 2> PointLookup;
typedef std::vector<PointLookup> PointLookupList;

class Laser
{
  public:
    Screen screen;
    float laserDelay = 25;
    bool optimizeOrder = false;
    bool toggleLaser = true;
    HardwareSerial *loggerPtr;

    Laser();
    Frame getDigit(int x, int height = 50, int width = 20);

    void drawPoint(int x, int y);
    void draw();
    long getClosestPoint(long currentIndex, std::vector<Point> & remaining);
};

#endif