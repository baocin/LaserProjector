#ifndef ScreenComponents_H
#define ScreenComponents_H

#include <vector>
#include <unordered_map>

//The most primitive primitive of the drawable image
class Point
{
  public:
    int x, y, z;
    int r, g, b;
    bool blanking; //true for laser on, false otherwise

    Point();
    Point(int x, int y, int z);
};

//A frame is a rectangular bounding box containing pixels
class Frame
{
  public:
    long id;
    std::vector<Point> points;
    int width, height;
    int x, y;

    Frame();
    void assignPoints(std::vector<Point> points);
    void addPoint(Point p);
    void draw(void (*drawPoint)(Point));
    void shift(int deltaX, int deltaY);
    void move(int newX, int newY);
    bool isPointInside(Point p);
};

//An analog to the entire visible
class Screen
{
  public:
    int width, height;

    //The current frame
    std::vector<long> visibleFrameIds;

    //A screen is a collection of frames, although each frame usually
    //replaces the previous each frame also has a x,y and height,width
    //allowing it to replace only a portion of the screen
    std::unordered_map<long, Frame> frames;

    Screen();
    
    void setFrames(std::vector<Frame> newFrames);
    void advanceStep(int stepCount = 1);
    void removeFrame(long id);
    std::vector<Frame> getVisibleFrames();
    void draw(void (*drawPoint)(Point));
    void addFrame(Frame frame, bool makeVisible);
};

#endif