#include <vector>
#include "ScreenComponents.h"

Point::Point(){

}

//, int r, int g, int b, bool blanking
Point::Point(int x, int y, int z){
    x = x;
    y = y;
    z = z;
    // r = r;
    // g = g;
    // b = b;
    // blanking = blanking;
}


Frame::Frame() {
    width = 255;
    height = 255;
}

void Frame::assignPoints(std::vector<Point> points) {
    points = points;
}

void Frame::addPoint(Point p){
    points.push_back(p);
}

void Frame::draw(void (*drawPoint)(Point)){
    for (long i; i < points.size(); i++){
        drawPoint(points[i]);
    }
}

void Frame::shift(int deltaX, int deltaY){
    for (long i; i < points.size(); i++){
        points[i].x += deltaX;
        points[i].x = points[i].x % width;

        points[i].y += deltaY;
        points[i].y = points[i].y % height;
    }
}

void Frame::move(int deltaX, int deltaY){
    //Modify the frame's position
    x += deltaX;
    x = x % width;

    y += deltaY;
    y = y % height;
}

void Screen::draw(void (*drawFrame)(Frame)){
    for (int i = 0; i < frames.size(); i++){
        drawFrame(frames[i]);
    }
}