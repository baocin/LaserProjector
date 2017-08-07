#include <vector>
#include <unordered_map>
#include <algorithm>

#include "ScreenComponents.h"

Point::Point() {
    x = 0;
    y = 0;
    z = 0;
    r = 0;
    g = 0;
    b = 0;
}

Point::Point(int x, int y, int z)
{
    int r, g, b;
    bool blanking;
    x = x;
    y = y;
    z = z;
    r = 255;
    g = 255;
    b = 255;
    blanking = false;
}

Point::Point(int x, int y, int z, int r, int g, int b)
{
    x = x;
    y = y;
    z = z;
    r = r;
    g = g;
    b = b;
    blanking = false;
}

Point::Point(int x, int y, int z, int r, int g, int b, bool blanking)
{
    x = x;
    y = y;
    z = z;
    r = r;
    g = g;
    b = b;
    blanking = blanking;
}

Frame::Frame()
{
    width = 255;
    height = 255;
}

void Frame::assignPoints(std::vector<Point> points)
{
    points = points;
}

void Frame::addPoint(Point p)
{
    points.push_back(p);
}

bool Frame::isPointInside(Point p)
{
    //Assume the provided point is after adding the frame's x and y to it
    if (p.x > (this->x + this->width) || p.y > (this->y + this->height))
    {
        return false;
    }
    if (p.x < this->x || p.y < this->y)
    {
        return false;
    }
    return true;
}

void Frame::draw(void (*drawPoint)(Point))
{
    for (long i = 0; i < points.size(); i++)
    {
        Point p = points[i];
        p.x += this->x;
        p.y += this->y;
        if (isPointInside(p))
        {
            drawPoint(p);
        }
    }
}

void Frame::shift(int deltaX, int deltaY)
{
    for (long i = 0; i < points.size(); i++)
    {
        points[i].x += deltaX;
        points[i].x = points[i].x % width;

        points[i].y += deltaY;
        points[i].y = points[i].y % height;
    }
}

void Frame::move(int newX, int newY)
{
    //Modify the frame's position
    x = newX;
    y = newY;
}

Screen::Screen() {}

std::vector<Frame> Screen::getVisibleFrames()
{
    std::vector<Frame> visibleFrames;
    for (long i = 0; i < visibleFrameIds.size(); i++)
    {
        visibleFrames.push_back(frames[visibleFrameIds[i]]);
    }
    return visibleFrames;
}
void Screen::removeFrame(long id)
{
    visibleFrameIds.erase(
        std::remove(visibleFrameIds.begin(), visibleFrameIds.end(), id),
        visibleFrameIds.end());
    frames.erase(id);
}

void Screen::advanceStep(int stepCount)
{
    if (visibleFrameIds.empty())
    {
        return;
    }
    for (long i = 0; i < visibleFrameIds.size(); i++)
    {
        long nextFrameId = visibleFrameIds[i] + stepCount;
        if (frames.find(nextFrameId) != frames.end())
        {
            //Found the next frame
            visibleFrameIds[i] = nextFrameId;
        }
    }
}

void Screen::draw(void (*drawPoint)(Point))
{
    for (long i = 0; i < visibleFrameIds.size(); i++)
    {
        frames[visibleFrameIds[i]].draw(drawPoint);
    }
}

void Screen::addFrame(Frame frame, bool makeVisible)
{
    frames.insert({{frame.id, frame}});
    if (makeVisible)
    {
        visibleFrameIds.push_back(frame.id);
    }
}

void Screen::clearVisibleFrames(){
    visibleFrameIds.clear();
}

void Screen::clearFrames(){
    frames.clear();
}