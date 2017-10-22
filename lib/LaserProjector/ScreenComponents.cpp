#include <vector>
#include <unordered_map>
#include <algorithm>
#include <Arduino.h>
#include <ArduinoLog.h>

#include "ScreenComponents.h"

Point::Point() {
    x = 0;
    y = 0;
    z = 0;
    r = 255;
    g = 255;
    b = 255;
    blanking = true;
}

Point::Point(int x, int y)
{
    Point();
    x = x;
    y = y;
}

Point::Point(int x, int y, int z)
{
    Point();
    x = x;
    y = y;
    z = z;
}

Point::Point(int x, int y, int z, int r, int g, int b)
{
    Point();
    x = x;
    y = y;
    z = z;
    r = r;
    g = g;
    b = b;
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

bool Point::operator==(const Point& rhs) const
{
    //Base equality off just the coordinate, not the color
    return (x == rhs.x && y == rhs.y && z == rhs.z);
}

bool Point::operator< (const Point &rhs) const
{
        return x < rhs.x;
}

Frame::Frame()
{
    width = 255;
    height = 255;
}

void Frame::assignPoints(std::vector<Point> points)
{
    changed = true;
    points = points;
}

void Frame::addPoint(Point p)
{
    changed = true;
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

void Frame::setPosition(int newX, int newY)
{
    //Modify the frame's position
    this -> x = newX;
    this -> y = newY;
}

void Frame::move(int newX, int newY)
{
    //Modify the frame's position
    this -> x = this -> x + newX;
    this -> y = this -> y + newY;
}

long Frame::getClosestPoint(long currentIndex, std::vector<Point> remaining){
    //Assumes remaining list is already sorted by X coordinate

    long closestIndex = currentIndex;  //index of the closest point in the remaining list
    long closestDistance = LONG_MAX;

    for (long p = currentIndex + 1, len = remaining.size(); p < len; p++ ){
    //for (long p = remaining.size(), len = remaining.size(); p > (currentIndex + 1); p-- ){
        //Ignore the sqrt function since it's redundant
        //long distance = abs((remaining[p].x - remaining[closestIndex].x) + (remaining[p].y - remaining[closestIndex].y));
        //(pow(remaining[p].x - remaining[closestIndex].x, 2) + pow(remaining[p].y - remaining[closestIndex].y, 2) );
        //abs(remaining[closestIndex].x - remaining[p].x) + abs(remaining[closestIndex].y - remaining[p].y);
        Point p1 = remaining[p];
        Point p2 = remaining[closestIndex];
        int distanceX = abs(p1.x - p2.x);
        int distanceY = abs(p1.y - p2.y);

        Log.notice("\t\t%d %d" CR, distanceX, distanceY);

        int distance = distanceX*distanceX + distanceY*distanceY;
        
        Log.notice("\t\tPoint id #%d: %d, %d, %d  - distance:%d" CR, p, remaining[p].x, remaining[p].y, remaining[p].z, distance);
        if (distance < closestDistance){
            Log.notice("\t\t\tFound closest point: %d, prev: %d" CR, distance, closestDistance);
            // Log.notice("\t\tPoint id #%d: %d, %d, %d" CR, p, remaining[p].x, remaining[p].y, remaining[p].z);
            closestDistance = distance;
            closestIndex = p;
        }
    }
    return closestIndex;
}

void Frame::removeDuplicatePoints(){
    //Remove duplicates
    points.erase(unique(points.begin(), points.end()), points.end());  
}

//Optimize the point list for drawing
void Frame::optimizePointOrder(){
    Log.notice("Optimizing point order for frame id #%d" CR, id);

    if (points.size() <= 0) return;

    Log.notice("Current num points = %d" CR, points.size());
    
    removeDuplicatePoints();
    Log.notice("Removed Duplicates, current num points = %d" CR, points.size());

    //Sort by x coordinate first
    std::sort(points.begin(), points.end());

    //Sort unique values for optimal drawing
    for (long i = 0, len = points.size() - 1; i < len; i++){
        Log.notice("Loop #%d" CR, i);
        Log.notice("\tPoint (%d) %d, %d, %d" CR, i, points[i].x, points[i].y, points[i].z);
        long nextIndex = getClosestPoint(i, points);
        Log.notice("\tReplacing id #%d with #%d" CR, i, nextIndex);
        Log.notice("\tPoint (%d) %d, %d, %d -> (%d) %d, %d, %d" CR, i+1, points[i+1].x, points[i+1].y, points[i+1].z, nextIndex, points[nextIndex].x, points[nextIndex].y, points[nextIndex].z);
        //swap next point with the found point
        std::iter_swap(points.begin() + i + 1,
                       points.begin() + nextIndex);
        Log.notice("\tPoint (%d) %d, %d, %d -> (%d) %d, %d, %d" CR, i+1, points[i+1].x, points[i+1].y, points[i+1].z, nextIndex, points[nextIndex].x, points[nextIndex].y, points[nextIndex].z);
        // Point temp = points[i+1];
        // points[i+1] = points[nextIndex];
        // points[nextIndex] = temp;

        // Log.notice("Swapping index values..." CR);
        // Log.notice("Point 1: %d, %d, %d" CR, points[i+1].x, points[i+1].y, points[i+1].z);
        // Log.notice("Point 2: %d, %d, %d" CR, points[nextIndex].x, points[nextIndex].y, points[nextIndex].z);
    }


    for (long i = 0, len = points.size() - 1; i < len; i++){
        Log.notice("Point %d: %d, %d, %d" CR, i, points[i].x, points[i].y, points[i].z);
    }
    changed = false;
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
    visibleFrameIds.clear();
}