/*********
   Simple Laser Projector
    Materials:
        Base: https://www.aliexpress.com/item/30K-laser-Galvo-Galvanometer-Based-Optical-Scanner-including-Show-Card/32531362627.html
            Includes 2 Galvanometer Optical Scanners
            Powersupply
            Logic board to do the heavy lifting
            The original control board and sd card are replaced with a DOIT ESP32 for more control.
        Laser: https://www.aliexpress.com/item/10pcs-lot-650nm-6mm-5V-5mW-Laser-Dot-Diode-Module-Copper-Head-Red/32583074085.html
            Simple, cheap, yet pretty visible at night or in shadow.
        (New) Control Board: https://www.aliexpress.com/item/Free-shipping-ESP32-Development-Board-Wifi-Bluetooth-Ultra-Low-Power-Consumption-Dual-Core-ESP-32-ESP/32803195605.html
            Cheap Wifi + Bluetooth with TWO cpu cores (insanity!)
        Proper DACs(Digital to Analog converters): https://www.aliexpress.com/item/MCP4725-I2C-DAC-Breakout-module-development-board/32311696869.html
            While waiting for china shipping I'm using two R/2R DAC Ladders for 8 bit precision of laser movement.
            Operation Amplifiers used for the R/2R DACs: https://www.aliexpress.com/item/Free-Shipping-10-pcs-DIP-IC-8pins-UA741CN-UA741CP-UA741-OP-Amp-LM741-741-TI-Original/2040771771.html 
            UPDATE: Now using the two built-in 8bit DACS on the ESP32 (GPIO 25 and 26)

    Plans (Roughly sorted from easiest to hardest):
        - Make web interface to control laser.
        - Make mobile app to control laser using bluetooth and cell phone accelerometer?
        - Draw text using laser
        - Recreate functionality of the original control board (namely reading ILDA files)
            - https://www.ilda.com/resources/StandardsDocs/ILDA_ISP99_rev002.pdf
        - Draw text
        - Scaling of the display
        - Fix angle hardware limitation (currently I'm only using a single quadrant[positive, positive] of the max rotation of the galvanometers)
        - Colors!
        - Noise reduction
        - Resolution options (skip ever x pixel, trade precision between x and y axis)
        - Machine learning + Computer Vision to outline objects with laser and provide additional information 
        - Perspective shift
            - Alter the perspective that the laser is draw on a surface to make it look like the laser was located somewhere else
            - Think of the perspective tool from Gimp or Photoshop to skew and warp images
            - Combine with computer vision to recognize the skew of a surface and automatically change the image being draw so it looks straight on the projected surface
        - External Monitor
            - Wayland driver support (lol!)

    Author: Michael Pedersen
*********/

#include <WiFi.h>
#include <driver/dac.h>
#include <vector>

#include "ScreenComponents.h"
#include "Laser.h"

using namespace std;

#define SERIAL_SPEED 115200
#define DAC_PIN_COUNT 8

// ILDAParser parser;
Laser laser;

void setup()
{
    //Setup Serial
    Serial.begin(SERIAL_SPEED);
    while (!Serial)
    {
        ; // wait for serial port to connect.
    }
    Serial.print("Setting serial speed to ");
    Serial.println(SERIAL_SPEED);

    //Setup Randomness
    Serial.println("Setting up Randomness");
    randomSeed(analogRead(0));

    //Setup Laser
    Frame newFrame = laser.getDigit(9, 50, 20);
    laser.screen.addFrame(newFrame, true);

    newFrame = laser.getDigit(7, 50, 20);
    newFrame.x += 20;
    laser.screen.addFrame(newFrame, true);

    newFrame = laser.getDigit(3, 50, 20);
    newFrame.x += 20;
    newFrame.y += 20;
    laser.screen.addFrame(newFrame, true);

    Serial.println(laser.screen.visibleFrameIds.size());
    for (int k = 0; k < laser.screen.visibleFrameIds.size(); k++){
        Serial.print(laser.screen.visibleFrameIds[k]);
        Serial.println(", ");
        for (int p = 0; p < laser.screen.frames[laser.screen.visibleFrameIds[k]].points.size(); p++){
            Serial.printf("Draw Point[%u,%u,%u] to frame\n", 
            laser.screen.frames[laser.screen.visibleFrameIds[k]].points[p].x,
            laser.screen.frames[laser.screen.visibleFrameIds[k]].points[p].y,
            laser.screen.frames[laser.screen.visibleFrameIds[k]].points[p].x
            );
        }
    }
    Serial.println();

    Serial.println("Setup Done");
}

void loop()
{
    laser.draw();
    laser.screen.frames[laser.screen.visibleFrameIds[0]].x += 1;
    laser.screen.frames[laser.screen.visibleFrameIds[0]].x = laser.screen.frames[laser.screen.visibleFrameIds[0]].x % 255;

    laser.screen.frames[laser.screen.visibleFrameIds[1]].y += 1;
    laser.screen.frames[laser.screen.visibleFrameIds[1]].y = laser.screen.frames[laser.screen.visibleFrameIds[0]].x % 255;
    
    laser.screen.frames[laser.screen.visibleFrameIds[2]].x += 1;
    laser.screen.frames[laser.screen.visibleFrameIds[2]].y += 1;
    laser.screen.frames[laser.screen.visibleFrameIds[2]].x = laser.screen.frames[laser.screen.visibleFrameIds[0]].x % 255;
    laser.screen.frames[laser.screen.visibleFrameIds[2]].y = laser.screen.frames[laser.screen.visibleFrameIds[0]].x % 255;
}