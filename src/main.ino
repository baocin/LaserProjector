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
#include <sstream>
#include <algorithm>

#include <ArduinoJson.h>

#include "ScreenComponents.h"
#include "Laser.h"
// #include "WiFiSimpleServer.h"

using namespace std;

#define SERIAL_SPEED 115200
#define DAC_PIN_COUNT 8

// ILDAParser parser;
Laser laser;

//Rest API
// WiFiSimpleServer server;
//Replace with environment variables
const char *ssid = "";
const char *password = "";

WiFiServer server(80);
// Client variables
char linebuf[1000]; //Might limit complexity of passed in JSON
int charcount = 0;
//The buffer size for JSON responses
const size_t bufferSize = JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(1) + JSON_OBJECT_SIZE(6) + 50;

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
    Frame newFrame = laser.getDigit(0, 50, 20);
    laser.screen.addFrame(newFrame, true);

    // newFrame = laser.getDigit(8, 50, 20);
    // newFrame.x += 20;
    // laser.screen.addFrame(newFrame, true);

    // newFrame = laser.getDigit(9, 50, 20);
    // newFrame.x += 20;
    // newFrame.y += 20;
    // laser.screen.addFrame(newFrame, true);

    // Serial.println(laser.screen.visibleFrameIds.size());
    // for (int k = 0; k < laser.screen.visibleFrameIds.size(); k++)
    // {
    //     Serial.print(laser.screen.visibleFrameIds[k]);
    //     Serial.println(", ");
    //     for (int p = 0; p < laser.screen.frames[laser.screen.visibleFrameIds[k]].points.size(); p++)
    //     {
    //         Serial.printf("Draw Point[%u,%u,%u] to frame\n",
    //                       laser.screen.frames[laser.screen.visibleFrameIds[k]].points[p].x,
    //                       laser.screen.frames[laser.screen.visibleFrameIds[k]].points[p].y,
    //                       laser.screen.frames[laser.screen.visibleFrameIds[k]].points[p].x);
    //     }
    // }
    // Serial.println();

    WiFi.begin(ssid, password);

    int delayCount = 0;
    // attempt to connect to Wifi network:
    while (WiFi.status() != WL_CONNECTED)
    {
        // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
        delay(500);
        Serial.print(".");
        delayCount++;
        if (delayCount > 5)
        {
            ESP.restart();
        }
    }
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

    server.begin();

    Serial.println("Setup Done");
}

void loop()
{
    processWebRequests();

    laser.draw();
    // laser.screen.frames[laser.screen.visibleFrameIds[0]].x += 1;
    // laser.screen.frames[laser.screen.visibleFrameIds[0]].x = laser.screen.frames[laser.screen.visibleFrameIds[0]].x % 255;

    // laser.screen.frames[laser.screen.visibleFrameIds[1]].y += 1;
    // laser.screen.frames[laser.screen.visibleFrameIds[1]].y = laser.screen.frames[laser.screen.visibleFrameIds[0]].x % 255;

    // laser.screen.frames[laser.screen.visibleFrameIds[2]].x += 1;
    // laser.screen.frames[laser.screen.visibleFrameIds[2]].y += 1;
    // laser.screen.frames[laser.screen.visibleFrameIds[2]].x = laser.screen.frames[laser.screen.visibleFrameIds[0]].x % 255;
    // laser.screen.frames[laser.screen.visibleFrameIds[2]].y = laser.screen.frames[laser.screen.visibleFrameIds[0]].x % 255;
}

bool parameterExists(String request, String key)
{
    return (request.indexOf(key + "="));
}

String parseParameter(String request, String key)
{
    String value;
    int keyIndex = request.indexOf(key) + key.length() + 1;
    int endIndex = request.indexOf("&", keyIndex);

    if (endIndex != -1)
    {
        //There is another parameter
        value = request.substring(keyIndex, endIndex).c_str();
    }
    else
    {
        //The end Index is ther end of the request string
        value = request.substring(keyIndex).c_str();
    }

    //Strip HTTP from the end
    int httpIndex = value.lastIndexOf("HTTP");
    value = value.substring(0, httpIndex);

    return value;
}

Frame parseFrameJson(String json)
{
    Frame resultFrame;
    //{"points": [{"x":100, "y":100, "z":0, "r":100,"g":100,"b":100}]}
    DynamicJsonBuffer jsonBuffer(bufferSize);

    //Remove HTML entities if there are any
    json.replace("%22", "\"");
    json.replace("%20", " ");

    Serial.println(json);
    JsonObject &root = jsonBuffer.parseObject(json);

    //Get the Frame's properties
    resultFrame.id = root["frameId"];
    resultFrame.x = root["x"];
    resultFrame.y = root["y"];
    resultFrame.width = root["width"];
    resultFrame.height = root["height"];

    //Start parsing points
    JsonArray &points = root["points"];
    for (int i = 0, len = points.size(); i < len; i++)
    {
        Serial.print("Parsing point ");
        Serial.println(i);
        // JsonObject &point = points[i];
        Point tempPoint; //TODO, add rgb
        
        tempPoint.x = points[i]["x"];
        tempPoint.y = points[i]["y"];
        tempPoint.z = points[i]["z"];
        tempPoint.r = points[i]["r"];
        tempPoint.g = points[i]["g"];
        tempPoint.b = points[i]["b"];
        tempPoint.blanking = points[i]["blanking"];
        // point.prettyPrintTo(Serial);

        resultFrame.addPoint(tempPoint);
        Serial.printf("Added point(%d,%d,%d,%d,%d,%d);", tempPoint.x, tempPoint.y, tempPoint.z, tempPoint.r, tempPoint.g, tempPoint.b);
    }

    return resultFrame;
}

void processWebRequests()
{
    // listen for incoming clients
    WiFiClient client = server.available();
    if (client)
    {
        String response;
        memset(linebuf, 0, sizeof(linebuf));
        charcount = 0;
        // an http request ends with a blank line
        boolean currentLineIsBlank = true;
        while (client.connected())
        {
            if (client.available())
            {
                char c = client.read();
                // Serial.write(c);
                //read char by char HTTP request
                linebuf[charcount] = c;
                if (charcount < sizeof(linebuf) - 1)
                    charcount++;
                // if you've gotten to the end of the line (received a newline
                // character) and the line is blank, the http request has ended,
                // so you can send a reply
                if (c == '\n' && currentLineIsBlank)
                {
                    // send a standard http response header
                    client.println("HTTP/1.1 200 OK");
                    client.println("Content-Type: text/html");
                    client.println("Connection: close"); // the connection will be closed after completion of the response
                    client.println();
                    if (response.length() == 0)
                    {
                        client.println("<!DOCTYPE HTML><html><head>");
                        client.println("<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"></head>");
                        client.println("<h1>ESP32 - Web Server</h1>");
                        client.println("<p>LED #1 <a href=\"on1\"><button>ON</button></a>&nbsp;<a href=\"off1\"><button>OFF</button></a></p>");
                        client.println("<p>LED #2 <a href=\"on2\"><button>ON</button></a>&nbsp;<a href=\"off2\"><button>OFF</button></a></p>");
                        client.println("</html>");
                    }
                    else
                    {
                        client.println(response);
                    }

                    break;
                }
                if (c == '\n')
                {
                    // you're starting a new line
                    currentLineIsBlank = true;

                    if (strstr(linebuf, "GET") > 0)
                    {
                        // Serial.println("Identified GET request");

                        if (strstr(linebuf, "/screen/visible/clear") > 0)
                        { //POST
                            //Clear all frames from visible frame list
                            laser.screen.clearVisibleFrames();
                        }else if (strstr(linebuf, "/laser/draw") > 0)
                        { //POST
                            laser.draw();
                        }
                        else if (strstr(linebuf, "/screen/frame/clear") > 0)
                        { //POST
                            //Clear all frames from the screen (including non visible)
                            laser.screen.clearVisibleFrames();
                            laser.screen.clearFrames();
                        }else if (strstr(linebuf, "/screen/visible/add") > 0)
                        {
                            long frameId;
                            if (parameterExists(linebuf, "frameId")){
                                frameId = parseParameter(linebuf, "frameId").toInt();
                                if (laser.screen.frames.count(frameId) != 0)
                                {
                                    //Frame exists, add it to visible frame list
                                    laser.screen.visibleFrameIds.push_back(frameId);
                                }
                            }
                        }else if (strstr(linebuf, "/screen/visible/remove") > 0)
                        {
                            long frameId;
                            if (parameterExists(linebuf, "frameId")){
                                frameId = parseParameter(linebuf, "frameId").toInt();
                                // if (laser.screen.frames.count(frameId) != 0)
                                // {
                                    //Frame exists, add it to visible frame list
                                    laser.screen.visibleFrameIds.erase(std::remove(laser.screen.visibleFrameIds.begin(), laser.screen.visibleFrameIds.end(), frameId), laser.screen.visibleFrameIds.end());
                                // }
                            }
                        }
                        else if (strstr(linebuf, "/screen/visible/frames") > 0)
                        {
                            //Get a json list of all visible frame ids
                            std::vector<Frame> frames = laser.screen.getVisibleFrames();

                            DynamicJsonBuffer jsonBuffer(bufferSize);

                            JsonObject &root = jsonBuffer.createObject();
                            JsonArray &frameIds = root.createNestedArray("frameIds");
                            for (int i = 0, len = frames.size(); i < len; i++)
                            {
                                frameIds.add(frames[i].id);
                            }
                            root.printTo(response);
                        }else if (strstr(linebuf, "/screen/frames") > 0)
                        {
                            //Get a json list of all visible frame ids
                            std::vector<Frame> frames;
                            frames.reserve(laser.screen.frames.size());
                            for (auto kv : laser.screen.frames){
                                frames.push_back(kv.second);
                            }

                            DynamicJsonBuffer jsonBuffer(bufferSize);

                            JsonObject &root = jsonBuffer.createObject();
                            JsonArray &frameIds = root.createNestedArray("frameIds");
                            for (int i = 0, len = frames.size(); i < len; i++)
                            {
                                frameIds.add(frames[i].id);
                            }
                            root.printTo(response);
                        }
                        else if (strstr(linebuf, "/frame/add") > 0)
                        {
                            //Get frame payload and add to visible frame
                            //payload is the frame data
                            //visible is bool for if it's currently visible
                            String payloadValue = parseParameter(linebuf, "payload");
                            Serial.print("Json");
                            Serial.println(payloadValue);
                            Frame newFrame = parseFrameJson(payloadValue);
                            bool isVisible = parameterExists(linebuf, "visible");
                            Serial.println(isVisible);
                            laser.screen.addFrame(newFrame, isVisible);
                        }
                        else if (strstr(linebuf, "/frame/delete") > 0)
                        {
                            //delete "frameId"
                            long frameId;
                            if (parameterExists(linebuf, "frameId")){
                                frameId = parseParameter(linebuf, "frameId").toInt();
                                laser.screen.removeFrame(frameId);
                            }
                        }
                        else if (strstr(linebuf, "/frame") > 0)
                        {
                            //TODO: Make middleware to check for required parameters and get their values.
                            long frameId = (parseParameter(linebuf, "frameId")).toInt();
                            DynamicJsonBuffer jsonBuffer(bufferSize);
                            JsonObject &root = jsonBuffer.createObject();

                            //Get a json list of all visible frame ids
                            if (laser.screen.frames.count(frameId) == 0)
                            {
                                //Couldn't find frameId in the frames list
                                root.printTo(response); //print empty JSON
                            }
                            else
                            {
                                Frame selectedFrame = laser.screen.frames[frameId];
                                root["frameId"] = selectedFrame.id;
                                root["x"] = selectedFrame.x;
                                root["y"] = selectedFrame.y;
                                root["width"] = selectedFrame.width;
                                root["height"] = selectedFrame.height;

                                //Read all the points from this frame
                                JsonArray &points = root.createNestedArray("points");
                                for (int i = 0, len = selectedFrame.points.size(); i < len; i++)
                                {
                                    JsonObject &point = jsonBuffer.createObject();
                                    point["x"] = selectedFrame.points[i].x;
                                    point["y"] = selectedFrame.points[i].y;
                                    point["z"] = selectedFrame.points[i].z;
                                    point["r"] = selectedFrame.points[i].r;
                                    point["g"] = selectedFrame.points[i].g;
                                    point["b"] = selectedFrame.points[i].b;
                                    point["blanking"] = selectedFrame.points[i].blanking;
                                    points.add(point);
                                }
                                root.printTo(response);
                            }
                        }
                        else if (strstr(linebuf, "/screen/seek") > 0)
                        {
                            //
                        }
                        else if (strstr(linebuf, "/screen/next") > 0)
                        {
                            //advanceStep of the visible frames
                            int stepCount = 1;
                            if (parameterExists(linebuf, "stepCount")){
                                stepCount = parseParameter(linebuf, "stepCount").toInt();
                            }
                            laser.screen.advanceStep(stepCount);
                        }
                        // else if (strstr(linebuf, "/frame/replace") > 0)
                        // {
                        //     //frameId is frame to replace
                        //     //payload is data to replace it with
                        // }
                        else if (strstr(linebuf, "/letter/add") > 0)
                        {
                            // laser.screen.removeFrame(laser.screen.visibleFrameIds[0]);
                            Frame newFrame = laser.getDigit(random(0, 9), random(30, 70), random(15, 25));
                            // Serial.printf("Frame ID: %d, Height:%d, Width:%d", newFrame.id, newFrame.height, newFrame.width);
                            laser.screen.removeFrame(newFrame.id);
                            laser.screen.addFrame(newFrame, true);
                            laser.screen.visibleFrameIds[newFrame.id] = newFrame.id;
                        }
                    }

                    // you're starting a new line
                    currentLineIsBlank = true;
                    memset(linebuf, 0, sizeof(linebuf));
                    charcount = 0;
                }
                else if (c != '\r')
                {
                    // you've gotten a character on the current line
                    currentLineIsBlank = false;
                }
            }
        }
        // give the web browser time to receive the data
        delay(1);

        // close the connection:
        client.stop();
    }
}
