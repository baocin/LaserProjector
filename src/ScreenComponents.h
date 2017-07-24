#include <vector>

class Point {
    public: 
        int x, y, z;
        int r, g, b;
        bool blanking;  //true for laser on, false otherwise

        Point();
        Point(int x, int y, int z);
};

class Frame {
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
        void move(int deltaX, int deltaY);

};

class Screen {
    //The current frame
    Frame frame;
    
    //A screen is a collection of frames, although each frame usually 
    //replaces the previous each frame also has a x,y and height,width
    //allowing it to replace only a portion of the screen
    std::vector<Frame> frames;
   
    void draw(void (*drawFrame)(Frame));

};