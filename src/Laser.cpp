#include <algorithm>
#include <driver/dac.h>
#include <Arduino.h>

#include "Laser.h"
#include "ScreenComponents.h"

//DAC Pins
const dac_channel_t DAC_X = DAC_CHANNEL_1; //Pin 25 by default for ESP32
const dac_channel_t DAC_Y = DAC_CHANNEL_2; //Pin 26 by default for ESP32

Laser::Laser()
{
    //Setup R/2R DAC for X coordinate
    dac_output_enable(DAC_X);

    //Setup R/2R DAC for Y Coordinate
    dac_output_enable(DAC_Y);

    setX(100);
    setY(100);
}

void Laser::drawPoint(Point p)
{
    //   Serial.printf("Draw Point[%u,%u,%u] to frame\n", p.x, p.y, p.z);
    setX(p.x);
    setY(p.y);

    //Delay ~500 microseconds
    //the average cycle counter per millisecond is 500,000
    //230000 is unreliable, 400,000 is reliable by
    //Reason: Motors need time to physically move into position.
        //TODO: modify delay so it is dependent on how much the motor has to move in both x and y axis (separately)
    uint32_t cycleCount = ESP.getCycleCount() + 250000;
    while (ESP.getCycleCount() < cycleCount){
        ;
    }
} 

void Laser::draw()
{
    this->screen.draw(this->drawPoint);
}

void Laser::setX(int x)
{
    //DAC_CHANNEL_1 is GPIO 25
    int status = dac_output_voltage(DAC_X, x);
    if (status == ESP_ERR_INVALID_ARG)
    {
        // Serial.print("Error setting X axis to ");
        // Serial.println(x);
    }
}

void Laser::setY(int y)
{
    //DAC_CHANNEL_2 is GPIO 26
    int status = dac_output_voltage(DAC_Y, y);
    if (status == ESP_ERR_INVALID_ARG)
    {
        // Serial.print("Error setting Y axis to ");
        // Serial.println(y);
    }
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