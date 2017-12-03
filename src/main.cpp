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

#include <ArduinoLog.h>

//To remove logging statements uncomment the following line
//#define DISABLE_LOGGING 

#define SERIAL_SPEED 115200 //921600
#define MAIN_SERIAL Serial

//Laser
Laser laser;
int laserPin = 27;

//Wifi
//const char *ssid = "MakerSpace Charlotte";
//const char *password = "MakeLearnShare1";
const char *ssid = "Colin & Michael's Bachelor Pad";
#const char *password = "";
// const char *ssid = "Laser Projector";
// const char *password = "projector";

WiFiMulti WiFiMulti;
ESP32WebServer server = ESP32WebServer(80);
WebSocketsServer webSocket = WebSocketsServer(81);

const char *indexHtml = "<html><head></head><body><h1>Access Laser control from separate website</h1></body></html>";

Frame parseFrameJson(String json)
{
    Frame resultFrame;

    // Assume 200 points
    DynamicJsonBuffer jsonBuffer(JSON_ARRAY_SIZE(200) + JSON_OBJECT_SIZE(6) + 200*JSON_OBJECT_SIZE(7));

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
    MAIN_SERIAL.print("Parsing ");
    MAIN_SERIAL.print(points.size());
    MAIN_SERIAL.println(" points");
    for (int i = 0, len = points.size(); i < len; i++)
    {
        Point tempPoint;

        tempPoint.x = points[i]["x"];
        tempPoint.y = points[i]["y"];
        tempPoint.z = points[i]["z"];
        tempPoint.r = points[i]["r"];
        tempPoint.g = points[i]["g"];
        tempPoint.b = points[i]["b"];
        tempPoint.blanking = points[i]["blanking"];

        resultFrame.addPoint(tempPoint);
        // MAIN_SERIAL.printf("Added point(%d,%d,%d,%d,%d,%d);\n", tempPoint.x, tempPoint.y, tempPoint.z, tempPoint.r, tempPoint.g, tempPoint.b);
        // MAIN_SERIAL.printf("Added point(%d,%d);\n", tempPoint.x, tempPoint.y);
    }

    
    return resultFrame;
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length)
{
    switch (type)
    {
    case WStype_DISCONNECTED:
        MAIN_SERIAL.printf("[%u] Disconnected!\n", num);
        webSocket.sendTXT(num, "{\"status\": \"Disconnected\"}");
        break;
    case WStype_CONNECTED:
    {
        IPAddress ip = webSocket.remoteIP(num);
        MAIN_SERIAL.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);

        // send message to client
        webSocket.sendTXT(num, "{\"status\": \"Connected\"}");
    }
    break;
    case WStype_TEXT:
        String text = (const char *)payload;
        MAIN_SERIAL.printf("[%u] get Text: %s\n", num, payload);

        DynamicJsonBuffer jsonBuffer(JSON_OBJECT_SIZE(5));
        JsonObject &root = jsonBuffer.parseObject(text);
        
        String command = root.get<String>("cmd");
        // MAIN_SERIAL.print("Command: ");
        // MAIN_SERIAL.println(command);

        long  fh = ESP.getFreeHeap();
        // MAIN_SERIAL.println(fh);
        
        if (command.compareTo("clearScreen") == 0){
            MAIN_SERIAL.println("Clear Screen");
            laser.screen.clearVisibleFrames();
            webSocket.sendTXT(num, "{\"mutation\":\"CLEAR_SCREEN\"}");
            
        }else if (command.compareTo("getRam") == 0){
            long  fh = ESP.getFreeHeap();
            MAIN_SERIAL.println(fh);
            JsonBuffer* jsonBufferRam = new StaticJsonBuffer<JSON_OBJECT_SIZE(2)>();
            JsonObject &rootRam = jsonBufferRam->createObject();
            rootRam["mutation"] = "RAM";
            String response;
            rootRam.printTo(response);
            webSocket.sendTXT(num, response);

            //webSocket.sendTXT(num, fh);
            webSocket.sendTXT(num, "{\"action\": \"getRam\"}");
        }else if (command.compareTo("clearFrames") == 0){
            MAIN_SERIAL.println("Clear Frames");
            laser.screen.clearFrames();
            webSocket.sendTXT(num, "{\"mutation\":\"CLEAR_FRAMES\"}");
        }else if (command.compareTo("deleteFrame") == 0){
            MAIN_SERIAL.println("Delete Frame");
            long frameId = root["frameId"];
            laser.screen.removeFrame(frameId);

            DynamicJsonBuffer jsonBufferDelete(JSON_ARRAY_SIZE(2));            
            JsonObject &rootDelete = jsonBufferDelete.createObject();
            rootDelete["mutation"] = "DELETE_FRAME";
            rootDelete["frameId"] = frameId;
            String response;
            rootDelete.printTo(response);
            webSocket.sendTXT(num, response);
        }else if (command.compareTo("clearFrame") == 0){
            MAIN_SERIAL.println("Clear Frame");
            long frameId = root["frameId"];
            laser.screen.frames[frameId].points.clear();
            laser.screen.frames[frameId].lastUpdate = millis();

            // Frame clearedFrame = laser.screen.frames[frameId];
            // clearedFrame.points.clear();
            // laser.screen.removeFrame(frameId);
            // laser.screen.addFrame(clearedFrame, true);

            DynamicJsonBuffer jsonBufferClear(JSON_ARRAY_SIZE(2));            
            JsonObject &rootClear = jsonBufferClear.createObject();
            rootClear["mutation"] = "CLEAR_FRAME";
            rootClear["frameId"] = frameId;
            String response;
            rootClear.printTo(response);
            webSocket.sendTXT(num, response);
        }else if (command.compareTo("addRandomFrame") == 0){
            // laser.screen.clearFrames();
            int a = random(0, 9);
            int b = random(20, 60);
            int c = random(15, 25);
            MAIN_SERIAL.printf("Laser Digit (%d, %d, %d)", a, b, c);
            Frame newFrame = laser.getDigit(a, 255, 255);
            laser.screen.addFrame(newFrame, true);
            laser.screen.visibleFrameIds[newFrame.id] = newFrame.id;
            //\"action\": \"addRandomFrame\"
            webSocket.sendTXT(num, "{}");
        }else if (command.compareTo("addFrame") == 0){
            MAIN_SERIAL.println("Add Frame");
            // MAIN_SERIAL.println(root["json"]);
            MAIN_SERIAL.flush();
            //Parse Frame JSON
            String rawJson = root.get<String>("json");
            
            MAIN_SERIAL.println(rawJson);
            Frame newFrame = parseFrameJson(rawJson);
            newFrame.lastUpdate = millis();
            laser.screen.addFrame(newFrame, true);
            webSocket.sendTXT(num, "{}");
        }else if (command.compareTo("addPoint") == 0){
            MAIN_SERIAL.println("Add Point");
            long frameId = root["frameId"];

            Point tempPoint;
            tempPoint.x = (long) root["x"];
            tempPoint.y = (long) root["y"];
            tempPoint.z = (long) root["z"];
            tempPoint.r = (long) root["r"];
            tempPoint.g = (long) root["g"];
            tempPoint.b = (long) root["b"];
            tempPoint.blanking = (bool) root["blanking"];

            laser.screen.frames[frameId].lastUpdate = millis();
            laser.screen.frames[frameId].addPoint(tempPoint);
            webSocket.sendTXT(num, "{}");
        }else if (command.compareTo("getVisibleFrameIds") == 0){
            MAIN_SERIAL.println("Get Visible Frame List");
            //Get a json list of all visible frame ids
            DynamicJsonBuffer jsonBufferVisibile(JSON_ARRAY_SIZE(10));            
            JsonObject &rootVisible = jsonBufferVisibile.createObject();
            JsonArray &frameIds = rootVisible.createNestedArray("frameIds");
            for (int i = 0, len = laser.screen.visibleFrameIds.size(); i < len; i++)
            {
                frameIds.add(laser.screen.visibleFrameIds[i]);
            }

            rootVisible["mutation"] = "VISIBLE_FRAME_IDS";

            String response;
            rootVisible.printTo(response);
            webSocket.sendTXT(num, response);
        }else if (command.compareTo("getFramesIds") == 0){
            // MAIN_SERIAL.println("Get All Frame List");
            // Assume 20 frames
            DynamicJsonBuffer jsonBuffer(JSON_ARRAY_SIZE(20) + JSON_OBJECT_SIZE(3) + JSON_OBJECT_SIZE(1));
            JsonObject &root = jsonBuffer.createObject();
            JsonArray &jsonFrameIds = root.createNestedArray("frameIds");
            for (auto kv : laser.screen.frames)
            {
                jsonFrameIds.add(kv.second.id);
            }
            root["mutation"] = "ALL_FRAME_IDS";
            String response;
            root.printTo(response);
            webSocket.sendTXT(num, response);
        }else if (command.compareTo("moveFrame") == 0){
            long frameId = root["frameId"];
            long x = root["x"];
            long y = root["y"];
            laser.screen.frames[frameId].move(x,y);
            webSocket.sendTXT(num, "{}");
        }else if (command.compareTo("setFrameVisibility") == 0){
            long frameId = root["frameId"];
            bool state = root["state"];
            laser.screen.setVisibility(frameId, state);
            webSocket.sendTXT(num, "{}");
        }else if (command.compareTo("getFrameInfo") == 0){
            long frameId = root["frameId"];
            long lastUpdate = root["lastUpdate"];
            
            // MAIN_SERIAL.print(frameId);
            // MAIN.SERIAL.print("   ");
            // MAIN_SERIAL.println(lastUpdate);
            //Get a json list of all visible frame ids
            if (laser.screen.frames.count(frameId) == 0)
            {
                MAIN_SERIAL.println("Cannot find frameId");
                //Couldn't find frameId in the frames list
                webSocket.sendTXT(num, "{}");
            }else if (lastUpdate == laser.screen.frames[frameId].lastUpdate){
                // MAIN_SERIAL.println("Frame hasn't been modified");
                webSocket.sendTXT(num, "{}");
            }
            else
            {
                // Assume 200 points
                DynamicJsonBuffer jsonBuffer(JSON_ARRAY_SIZE(200) + JSON_OBJECT_SIZE(8) + 200*JSON_OBJECT_SIZE(7));
                JsonObject &newJson = jsonBuffer.createObject();

                Frame selectedFrame = laser.screen.frames[frameId];
                newJson["frameId"] = selectedFrame.id;
                newJson["x"] = selectedFrame.x;
                newJson["y"] = selectedFrame.y;
                newJson["width"] = selectedFrame.width;
                newJson["height"] = selectedFrame.height;
                newJson["height"] = selectedFrame.height;
                newJson["lastUpdate"] = selectedFrame.lastUpdate;
                newJson["numPoints"] = selectedFrame.points.size();
                newJson["visible"] = laser.screen.isVisible(selectedFrame.id);

                //Read all the points from this frame
                JsonArray &points = newJson.createNestedArray("points");
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

                newJson["mutation"] = "FRAME_INFO";

                String response;
                newJson.printTo(response);
                MAIN_SERIAL.println(response);
                webSocket.sendTXT(num, response);
            }
        }else if (command.compareTo("drawScreen") == 0){
            MAIN_SERIAL.println("Drawing screen");
            laser.draw();
            webSocket.sendTXT(num, "{}");
        }else if (command.compareTo("laserDelay") == 0){
            MAIN_SERIAL.println("Setting the laser delay");
            float duration = root["duration"];
            laser.laserDelay = duration;
            webSocket.sendTXT(num, "{}");
        // }else if (command.compareTo("optimizeOrder") == 0){
        //     laser.optimizeOrder = !laser.optimizeOrder;
        //     MAIN_SERIAL.printf("optimize Order: %i", laser.optimizeOrder);
        //     // webSocket.sendTXT(num, laser.optimizeOrder);
        //     webSocket.sendTXT(num, "{\"action\": \"optimizeOrder\"}");
        }else if (command.compareTo("toggleLaser") == 0){
            laser.toggleLaser = !laser.toggleLaser;
            MAIN_SERIAL.printf("toggleLaser: %i", laser.toggleLaser);
            // webSocket.sendTXT(num, laser.toggleLaser);
            webSocket.sendTXT(num, "{}");
        }else if (command.compareTo("optimizePointOrder") == 0){
            long frameId = root["frameId"];
            laser.screen.frames[frameId].optimizePointOrder();
            webSocket.sendTXT(num, "{}");
        }else if (command.compareTo("restart") == 0){
            MAIN_SERIAL.println("Restart requested...");
            esp_restart();
        }else{
            MAIN_SERIAL.println("No Valid command found");
            webSocket.sendTXT(num, "{}");
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

    //Setup Logging
    Log.begin(LOG_LEVEL_VERBOSE, &MAIN_SERIAL);
    Log.notice("Setting serial speed to %d" CR, SERIAL_SPEED);
    
    //Setup Random
    Log.notice("Setting up Randomness" CR);
    randomSeed(analogRead(0));

    //Setup Laser
    Log.notice("Setting up Laser" CR);
    pinMode(laserPin, OUTPUT);
    digitalWrite(laserPin, LOW);
    
    //Setup Wifi
    // WiFi.softAP(ssid, password);
    WiFiMulti.addAP(ssid, password);
    while (WiFiMulti.run() != WL_CONNECTED)
    {
        delay(100);
    }
    MAIN_SERIAL.println("");
    Log.notice("WiFi connected" CR);
    Log.notice("IP address: %s"  CR, WiFi.localIP().toString());
    // Log.notice(WiFi.localIP());
    Serial.println(WiFi.localIP());

    // start webSocket server
    Log.notice("Starting WebSocket Server" CR);
    webSocket.begin();
    webSocket.onEvent(webSocketEvent);
    Log.notice("Websockets started" CR);

    if (MDNS.begin("esp8266"))
    {
        Log.notice("MDNS responder started" CR);
    }

    Log.notice("Setting up Http REST endpoints" CR);

    // handle index
    //TODO: Load from Filesystem
    server.on("/", []() {
        // send index.html
        server.send(200, "text/html", indexHtml);
    });

    Log.notice("Http WebServer started." CR);
    server.begin();

    // Add service to MDNS
    MDNS.addService("http", "tcp", 80);
    MDNS.addService("ws", "tcp", 81);

    Log.notice("Setup Done." CR);
}

void loop()
{
    laser.draw();
    webSocket.loop();
    // server.handleClient();
}
