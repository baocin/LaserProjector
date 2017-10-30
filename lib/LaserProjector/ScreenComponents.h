#ifndef ScreenComponents_H
#define ScreenComponents_H

#include <vector>
#include <unordered_map>

//todo: typedef void (*drawPoint)(Point);

//The most primitive primitive of the drawable image
class Point
{
  public:
    int x, y, z;
    int r, g, b;
    bool blanking; //true for turn laser on only when drawing the point, otherwise just move to the point

    Point();
    Point(int x, int y);
    Point(int x, int y, int z);
    Point(int x, int y, int z, int r, int g, int b);
    Point(int x, int y, int z, int r, int g, int b, bool blanking);
    bool operator==(const Point& rhs) const;
    bool operator< (const Point &rhs) const;
    
};

//A frame is a rectangular bounding box containing pixels
class Frame
{
  public:
    long id;
    std::vector<Point> points;
    int width, height;
    int x, y;
    long lastUpdate;
    
    Frame();
    void assignPoints(std::vector<Point> points);
    void addPoint(Point p);
    void shift(int deltaX, int deltaY);
    void move(int newX, int newY);
    void setPosition(int newX, int newY);
    bool isPointInside(Point p);
    void optimizePointOrder();
    long getClosestPoint(long currentIndex, std::vector<Point> remaining);
    void removeDuplicatePoints();
};

//An analog to the entire visible vasting area
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
    void addFrame(Frame frame, bool makeVisible);
    void clearVisibleFrames();
    void clearFrames();
    bool hasFrame(long frameId);
    void setVisibility(long frameId, bool isVisible);
    bool isVisible(long frameId);
};

#endif