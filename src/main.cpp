/*
 * WebSocketServer_LEDcontrol.ino
 *
 *  Created on: 26.11.2015
 *
 */

#include <Arduino.h>

#include <WiFi.h>
#include <WiFiMulti.h>
#include <WebSocketsServer.h>
#include <ESP32WebServer.h>
#include <ESPmDNS.h>
#include <Hash.h>

#include <ArduinoJson.h>

#include <Laser.h>
#include <ScreenComponents.h>

#define SERIAL_SPEED 115200 //921600
#define MAIN_SERIAL Serial

//Laser
Laser laser;

//Json
const size_t bufferSize = JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(1) + JSON_OBJECT_SIZE(6) + 50;

//Wifi
const char *ssid = "";
const char *password = "";
WiFiMulti WiFiMulti;
ESP32WebServer server = ESP32WebServer(80);
WebSocketsServer webSocket = WebSocketsServer(81);

//.toString(16);
const char *indexHtml =
    "<html><head><script>"
    "var connection = new WebSocket('ws://'+location.hostname+':81/', ['arduino']);"
    "connection.onopen = function () {connection.send('Connect ' + new Date()); };"
    "connection.onerror = function (error) {console.log('WebSocket Error ', error);};"
    "connection.onmessage = function (e) { console.log('Server: ', e.data);};"
    "function sendXY() {"
    "    var x = parseInt(document.getElementById('x').value);"
    "    var y = parseInt(document.getElementById('y').value);"
    ""
    "           var msg = {x : x, y : y};"
    "          console.log(msg);"
    "         connection.send(JSON.stringify(msg));"
    "    };"
    "   </script>"
    "  </head>"
    " <body>"
    "LED Control:"
    "<br/>"
    "<br/>"
    "X: <input id=\"x\" type=\"range\" min=\"0\" max=\"255\" step=\"1\" oninput=\"sendXY();\" /><br/>"
    "Y: <input id=\"y\" type=\"range\" min=\"0\" max=\"255\" step=\"1\" oninput=\"sendXY();\" />"
    "<br/>"
    ""
    "</body></html>";

Frame parseFrameJson(String json)
{
    Frame resultFrame;
    //{"points": [{"x":100, "y":100, "z":0, "r":100,"g":100,"b":100}]}
    MAIN_SERIAL.print("bufferSize: " );
    MAIN_SERIAL.println(bufferSize);
    DynamicJsonBuffer jsonBuffer(bufferSize);

    JsonObject &root = jsonBuffer.parseObject(json);

    root.printTo(MAIN_SERIAL);
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
        MAIN_SERIAL.print("Parsing point ");
        MAIN_SERIAL.println(i);
        // JsonObject &point = points[i];
        Point tempPoint; //TODO, add rgb

        tempPoint.x = points[i]["x"];
        tempPoint.y = points[i]["y"];
        // tempPoint.z = points[i]["z"];
        // tempPoint.r = points[i]["r"];
        // tempPoint.g = points[i]["g"];
        // tempPoint.b = points[i]["b"];
        // tempPoint.blanking = points[i]["blanking"];

        resultFrame.addPoint(tempPoint);
        // MAIN_SERIAL.printf("Added point(%d,%d,%d,%d,%d,%d);\n", tempPoint.x, tempPoint.y, tempPoint.z, tempPoint.r, tempPoint.g, tempPoint.b);
        MAIN_SERIAL.printf("Added point(%d,%d);\n", tempPoint.x, tempPoint.y);
    }

    
    return resultFrame;
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length)
{
    switch (type)
    {
    case WStype_DISCONNECTED:
        MAIN_SERIAL.printf("[%u] Disconnected!\n", num);
        break;
    case WStype_CONNECTED:
    {
        IPAddress ip = webSocket.remoteIP(num);
        MAIN_SERIAL.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);

        // send message to client
        webSocket.sendTXT(num, "Connected");
    }
    break;
    case WStype_TEXT:
        String text = (const char *)payload;
        MAIN_SERIAL.printf("[%u] get Text: %s\n", num, payload);

        DynamicJsonBuffer jsonBuffer(bufferSize);
        JsonObject &root = jsonBuffer.parseObject(text);
        
        String command = root.get<String>("cmd");
        MAIN_SERIAL.print("Command: ");
        MAIN_SERIAL.println(command);

        long  fh = ESP.getFreeHeap();
            MAIN_SERIAL.println(fh);
        
        if (command.compareTo("clearScreen") == 0){
            MAIN_SERIAL.println("Clear Screen");
            laser.screen.clearVisibleFrames();
        }else if (command.compareTo("getRam") == 0){
            long  fh = ESP.getFreeHeap();
            MAIN_SERIAL.println(fh);
            // webSocket.sendTXT(num, fh);
        }else if (command.compareTo("clearFrames") == 0){
            MAIN_SERIAL.println("Clear Frames");
            laser.screen.clearFrames();
        }else if (command.compareTo("addRandomFrame") == 0){
            // laser.screen.clearFrames();
            Frame newFrame = laser.getDigit(random(0, 9), random(30, 70), random(15, 25));
            laser.screen.removeFrame(newFrame.id);
            laser.screen.addFrame(newFrame, true);
            laser.screen.visibleFrameIds[newFrame.id] = newFrame.id;
        }else if (command.compareTo("addFrame") == 0){
            MAIN_SERIAL.println("Add Frame");
            //Parse Frame JSON
            String rawJson = root.get<String>("json");

            MAIN_SERIAL.println(rawJson);
            Frame newFrame = parseFrameJson(rawJson);
            laser.screen.addFrame(newFrame, true);
        }else if (command.compareTo("addPoint") == 0){
            MAIN_SERIAL.println("Add Point");
            long frameId = root["frameId"];
            long x = root["x"];
            long y = root["y"];
            Point tempPoint;
            tempPoint.x = x;
            tempPoint.y = y;
            laser.screen.frames[frameId].addPoint(tempPoint);
            
        }else if (command.compareTo("getVisibleFrames") == 0){
            MAIN_SERIAL.println("Get Visible Frame List");
            //Get a json list of all visible frame ids
            DynamicJsonBuffer jsonBuffer(bufferSize);
            JsonObject &root = jsonBuffer.createObject();
            JsonArray &frameIds = root.createNestedArray("frameIds");
            for (int i = 0, len = laser.screen.visibleFrameIds.size(); i < len; i++)
            {
                frameIds.add(laser.screen.visibleFrameIds[i]);
            }

            String response;
            root.printTo(response);
            webSocket.sendTXT(num, response);
        }else if (command.compareTo("getFrames") == 0){
            MAIN_SERIAL.println("Get All Frame List");
            //Get a json list of all visible frame ids
            std::vector<Frame> frames;
            frames.reserve(laser.screen.frames.size());
            for (auto kv : laser.screen.frames)
            {
                frames.push_back(kv.second);
            }

            DynamicJsonBuffer jsonBuffer(bufferSize);
            JsonObject &root = jsonBuffer.createObject();
            JsonArray &frameIds = root.createNestedArray("frameIds");
            for (int i = 0, len = frames.size(); i < len; i++)
            {
                frameIds.add(frames[i].id);
            }

            String response;
            root.printTo(response);
            webSocket.sendTXT(num, response);
        }else if (command.compareTo("moveFrame") == 0){
            long frameId = root["frameId"];
            long x = root["x"];
            long y = root["y"];
            laser.screen.frames[frameId].move(x,y);

        }else if (command.compareTo("getFrameInfo") == 0){
            long frameId = root["frameId"];
            JsonObject &newJson = jsonBuffer.createObject();

            //Get a json list of all visible frame ids
            if (laser.screen.frames.count(frameId) == 0)
            {
                //Couldn't find frameId in the frames list
                webSocket.sendTXT(num, "{}");
                // root.printTo(response); //print empty JSON
            }
            else
            {
                Frame selectedFrame = laser.screen.frames[frameId];
                newJson["frameId"] = selectedFrame.id;
                newJson["x"] = selectedFrame.x;
                newJson["y"] = selectedFrame.y;
                newJson["width"] = selectedFrame.width;
                newJson["height"] = selectedFrame.height;

                //Read all the points from this frame
                JsonArray &points = newJson.createNestedArray("points");
                for (int i = 0, len = selectedFrame.points.size(); i < len; i++)
                {
                    JsonObject &point = jsonBuffer.createObject();
                    point["x"] = selectedFrame.points[i].x;
                    point["y"] = selectedFrame.points[i].y;
                    // point["z"] = selectedFrame.points[i].z;
                    // point["r"] = selectedFrame.points[i].r;
                    // point["g"] = selectedFrame.points[i].g;
                    // point["b"] = selectedFrame.points[i].b;
                    // point["blanking"] = selectedFrame.points[i].blanking;
                    points.add(point);
                }

                String response;
                newJson.printTo(response);
                webSocket.sendTXT(num, response);
            }
        }else if (command.compareTo("addPoint") == 0){
            MAIN_SERIAL.println("Add Point");
        }else if (command.compareTo("drawScreen") == 0){
            MAIN_SERIAL.println("Drawing screen");
            laser.draw();
        }else{
            MAIN_SERIAL.println("No Valid command found");
        }
    }
}

void setup()
{
    MAIN_SERIAL.begin(SERIAL_SPEED);
    while (!MAIN_SERIAL)
    {
        ; // wait for serial port to connect.
    }

    MAIN_SERIAL.println();
    MAIN_SERIAL.println();
    MAIN_SERIAL.println();

    MAIN_SERIAL.setDebugOutput(true);
    MAIN_SERIAL.print("Setting serial speed to ");
    MAIN_SERIAL.println(SERIAL_SPEED);

    //Setup Random
    MAIN_SERIAL.println("Setting up Randomness");
    randomSeed(analogRead(0));

    //Setup Laser
    MAIN_SERIAL.println("Setting up Laser");
    
    //Setup Wifi
    WiFiMulti.addAP(ssid, password);
    while (WiFiMulti.run() != WL_CONNECTED)
    {
        delay(100);
    }
    MAIN_SERIAL.println("");
    MAIN_SERIAL.println("WiFi connected");
    MAIN_SERIAL.println("IP address: ");
    MAIN_SERIAL.println(WiFi.localIP());

    // start webSocket server
    MAIN_SERIAL.println("Starting WebSocket Server");
    webSocket.begin();
    webSocket.onEvent(webSocketEvent);
    MAIN_SERIAL.println("Websockets started");

    if (MDNS.begin("esp8266"))
    {
        MAIN_SERIAL.println("MDNS responder started");
    }

    MAIN_SERIAL.println("Setting up Http REST endpoints");
    // handle index
    server.on("/", []() {
        // send index.html
        server.send(200, "text/html", indexHtml);
    });

    MAIN_SERIAL.println("Http WebServer started.");
    server.begin();

    // Add service to MDNS
    MDNS.addService("http", "tcp", 80);
    MDNS.addService("ws", "tcp", 81);

    MAIN_SERIAL.println("Setup Done.");
}

void loop()
{
    laser.draw();
    webSocket.loop();
    server.handleClient();
}
