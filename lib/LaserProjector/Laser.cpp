#include <algorithm>
#include <driver/dac.h>
#include <Arduino.h>
#include <algorithm>
#include <Wire.h>
#include <ArduinoLog.h>
// #include <Adafruit_MCP4725.h>

#include "Laser.h"
#include "ScreenComponents.h"

//DAC Pins
const dac_channel_t DAC_X = DAC_CHANNEL_1; //Pin 25 by default for ESP32
const dac_channel_t DAC_Y = DAC_CHANNEL_2; //Pin 26 by default for ESP32
const int LASER_PIN = 27;

// Adafruit_MCP4725 DACX;
// Adafruit_MCP4725 DACY;

Laser::Laser()
{
    //Setup R/2R DAC for X coordinate
    dac_output_enable(DAC_X);

    //Setup R/2R DAC for Y Coordinate
    dac_output_enable(DAC_Y);

    //Will be used when converted to use 12 bit instead of the internal 8 bit DACs
    // Wire.begin();
    // DACX.begin(0x60);
    // DACY.begin(0x61);
    // // Set A2 and A3 as Outputs to make them our GND and Vcc,
    // //which will power the MCP4725
    // pinMode(A2, OUTPUT);
    // pinMode(A3, OUTPUT);
    // digitalWrite(A2, LOW);//Set A2 as GND
    // digitalWrite(A3, HIGH);//Set A3 as Vcc

    pinMode(LASER_PIN, OUTPUT);
}

void Laser::draw()
{
    // for (long f = 0; f < this -> screen.visibleFrameIds.size(); f++)
    // {
    if (this -> screen.visibleFrameIds.size() <= 0) return;

    long frameId = this -> screen.visibleFrameIds[0];

    if (this -> screen.frames[frameId].changed && optimizeOrder){
        //Ro-optimize frame
        // if (optimizeOrder){
           this -> screen.frames[frameId].optimizePointOrder();
        // }
    }
    
    Point previousPoint;
    for (long p = 0, len = this -> screen.frames[frameId].points.size(); p < len; p++)
    {
        Point point = this -> screen.frames[frameId].points[p];
        point.x += this -> screen.frames[frameId].x;
        point.y += this -> screen.frames[frameId].y;

        if (this -> screen.frames[frameId].isPointInside(point))
        {
            //Move to Position
            drawPoint(point.x, point.y);
            //
            // delayMicroseconds(50);

            if(toggleLaser)
                digitalWrite(LASER_PIN, LOW);

            //Delay 1000 microseconds for motors to physically move into position.
            //TODO: modify delay so it is dependent on how much the motor has to move in both x and y axis (separately)
            //Max: 16383
            //delayMicroseconds(10);
            float distance = abs(point.x - previousPoint.x) + abs(point.y - previousPoint.y);
            //Log.notice("Current Point (%d,%d)  -  Previous Point (%d,%d)" CR, point.x,point.y, previousPoint.x, previousPoint.y);
            delayMicroseconds(distance * laserDelay);

            if(toggleLaser)
                digitalWrite(LASER_PIN, HIGH);
        }

        previousPoint = point;
    }
}

void Laser::drawPoint(int x, int y)
{
    dac_output_voltage(DAC_X, x);
    dac_output_voltage(DAC_Y, y);
}

Frame Laser::getDigit(int x, int height, int width)
{
    Frame frame;
    frame.id = x;
    frame.height = height;
    frame.width = width;
    frame.x = 0;
    frame.y = 0;
    Point a;

    //1
    if (x == 0 || x == 1 || x == 2 || x == 3 || x == 4 || x == 5 || x == 6 || x == 7 || x == 8 || x == 9)
    {
        a.x = 1 * frame.width;
        a.y = 1 * frame.height;
        frame.addPoint(a);
    }
    //2
    if (x == 0 || x == 1 || x == 2 || x == 3 || x == 5 || x == 6 || x == 7 || x == 8 || x == 9)
    {
        a.x = .5 * frame.width;
        a.y = 1 * frame.height;
        frame.addPoint(a);
    }
    //3
    if (x == 0 || x == 2 || x == 3 || x == 4 || x == 5 || x == 6 || x == 7 || x == 8 || x == 9)
    {
        a.x = 0;
        a.y = 1 * frame.height;
        frame.addPoint(a);
    }
    //4
    if (x == 0 || x == 4 || x == 5 || x == 6 || x == 8 || x == 9)
    {
        a.x = 1 * frame.width;
        a.y = .75 * frame.height;
        frame.addPoint(a);
    }
    //5
    if (x == 1)
    {
        a.x = .5 * frame.width;
        a.y = .75 * frame.height;
        frame.addPoint(a);
    }
    //6
    if (x == 0 || x == 2 || x == 3 || x == 4 || x == 7 || x == 8 || x == 9)
    {
        a.x = 0;
        a.y = .75 * frame.height;
        frame.addPoint(a);
    }
    //7
    if (x == 0 || x == 2 || x == 3 || x == 4 || x == 5 || x == 6 || x == 8 || x == 9)
    {
        a.x = 1 * frame.width;
        a.y = .5 * frame.height;
        frame.addPoint(a);
    }
    //8
    if (x == 1 || x == 2 || x == 3 || x == 4 || x == 5 || x == 6 || x == 8 || x == 9)
    {
        a.x = .5 * frame.width;
        a.y = .5 * frame.height;
        frame.addPoint(a);
    }
    //9
    if (x == 0 || x == 2 || x == 3 || x == 4 || x == 5 || x == 6 || x == 7 || x == 8 || x == 9)
    {
        a.x = 0;
        a.y = .5 * frame.height;
        frame.addPoint(a);
    }
    //10
    if (x == 0 || x == 2 || x == 6 || x == 8)
    {
        a.x = 1 * frame.width;
        a.y = .25 * frame.height;
        frame.addPoint(a);
    }
    //11
    if (x == 1)
    {
        a.x = .5 * frame.width;
        a.y = .25 * frame.height;
        frame.addPoint(a);
    }
    //12
    if (x == 0 || x == 3 || x == 4 || x == 5 || x == 6 || x == 7 || x == 8 || x == 9)
    {
        a.x = 0;
        a.y = .25 * frame.height;
        frame.addPoint(a);
    }
    //13
    if (x == 0 || x == 1 || x == 2 || x == 3 || x == 5 || x == 6 || x == 8)
    {
        a.x = 1 * frame.width;
        a.y = 0;
        frame.addPoint(a);
    }
    //14
    if (x == 0 || x == 1 || x == 2 || x == 3 || x == 5 || x == 6 || x == 8)
    {
        a.x = .5 * frame.width;
        a.y = 0;
        frame.addPoint(a);
    }
    //15
    if (x == 0 || x == 1 || x == 2 || x == 3 || x == 4 || x == 5 || x == 6 || x == 7 || x == 8 || x == 9)
    {
        a.x = 0;
        a.y = 0;
        frame.addPoint(a);
    }

    return frame;
}