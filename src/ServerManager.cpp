#include "ServerManager.h"
#include "Globals.h"
#include <WebServer.h>
#include <esp-fs-webserver.h>
#include "htmls.h"
#include <ESPmDNS.h>
#include <LittleFS.h>
#include <WiFi.h>
#include "DisplayManager.h"
#include "PeripheryManager.h"
#include "PowerManager.h"
#include <WiFiUdp.h>
#include <HTTPClient.h>
#include "GameManager.h"

WiFiUDP udp;
unsigned int localUdpPort = 4210; // Wählen Sie einen Port
char incomingPacket[255];

WebServer server(80);
FSWebServer mws(LittleFS, server);

// The getter for the instantiated singleton instance
ServerManager_ &ServerManager_::getInstance()
{
    static ServerManager_ instance;
    return instance;
}

// Initialize the global shared instance
ServerManager_ &ServerManager = ServerManager.getInstance();

void versionHandler()
{
    WebServerClass *webRequest = mws.getRequest();
    webRequest->send(200, F("text/plain"), VERSION);
}

void saveHandler()
{
    WebServerClass *webRequest = mws.getRequest();
    ServerManager.getInstance().loadSettings();
    webRequest->send(200);
}

void addHandler()
{
    mws.addHandler("/api/power", HTTP_POST, []()
                   { DisplayManager.powerStateParse(mws.webserver->arg("plain").c_str()); mws.webserver->send(200,F("text/plain"),F("OK")); });
    mws.addHandler(
        "/api/sleep", HTTP_POST, []()
        { 
            mws.webserver->send(200,F("text/plain"),F("OK"));
            DisplayManager.setPower(false);
            PowerManager.sleepParser(mws.webserver->arg("plain").c_str()); });
    mws.addHandler("/api/loop", HTTP_GET, []()
                   { mws.webserver->send_P(200, "application/json", DisplayManager.getAppsAsJson().c_str()); });
    mws.addHandler("/api/effects", HTTP_GET, []()
                   { mws.webserver->send_P(200, "application/json", DisplayManager.getEffectNames().c_str()); });
    mws.addHandler("/api/transitions", HTTP_GET, []()
                   { mws.webserver->send_P(200, "application/json", DisplayManager.getTransitionNames().c_str()); });
    mws.addHandler("/api/reboot", HTTP_ANY, []()
                   { mws.webserver->send(200,F("text/plain"),F("OK")); delay(200); ESP.restart(); });
    mws.addHandler("/api/rtttl", HTTP_POST, []()
                   { mws.webserver->send(200,F("text/plain"),F("OK")); PeripheryManager.playRTTTLString(mws.webserver->arg("plain").c_str()); });
    mws.addHandler("/api/sound", HTTP_POST, []()
                   { if (PeripheryManager.parseSound(mws.webserver->arg("plain").c_str())){
                    mws.webserver->send(200,F("text/plain"),F("OK")); 
                   }else{
                    mws.webserver->send(404,F("text/plain"),F("FileNotFound"));  
                   }; });

    mws.addHandler("/api/moodlight", HTTP_POST, []()
                   {
                    if (DisplayManager.moodlight(mws.webserver->arg("plain").c_str()))
                    {
                        mws.webserver->send(200, F(F("text/plain")), F("OK"));
                    }
                    else
                    {
                        mws.webserver->send(500, F("text/plain"), F("ErrorParsingJson"));
                    } });
    mws.addHandler("/api/notify", HTTP_POST, []()
                   {
                       if (DisplayManager.generateNotification(1,mws.webserver->arg("plain").c_str()))
                       {
                        mws.webserver->send(200, F("text/plain"), F("OK"));
                       }else{
                        mws.webserver->send(500, F("text/plain"), F("ErrorParsingJson"));
                       } });
    mws.addHandler("/api/nextapp", HTTP_ANY, []()
                   { DisplayManager.nextApp(); mws.webserver->send(200,F("text/plain"),F("OK")); });
    mws.addHandler("/fullscreen", HTTP_GET, []()
                   {
    String fps = mws.webserver->arg("fps");
    if (fps == "") {
        fps = "30"; 
    }
    String finalHTML = screenfull_html; 
    finalHTML.replace("%%FPS%%", fps);

    mws.webserver->send(200, "text/html", finalHTML.c_str()); });
    mws.addHandler("/screen", HTTP_GET, []()
                   { mws.webserver->send(200, "text/html", screen_html); });
    mws.addHandler("/backup", HTTP_GET, []()
                   { mws.webserver->send(200, "text/html", backup_html); });
    mws.addHandler("/api/previousapp", HTTP_POST, []()
                   { DisplayManager.previousApp(); mws.webserver->send(200,F("text/plain"),F("OK")); });
    mws.addHandler("/api/notify/dismiss", HTTP_ANY, []()
                   { DisplayManager.dismissNotify(); mws.webserver->send(200,F("text/plain"),F("OK")); });
    mws.addHandler("/api/apps", HTTP_POST, []()
                   { DisplayManager.updateAppVector(mws.webserver->arg("plain").c_str()); mws.webserver->send(200,F("text/plain"),F("OK")); });
    mws.addHandler(
        "/api/switch", HTTP_POST, []()
        {
        if (DisplayManager.switchToApp(mws.webserver->arg("plain").c_str()))
        {
            mws.webserver->send(200, F("text/plain"), F("OK"));
        }
        else
        {
            mws.webserver->send(500, F("text/plain"), F("FAILED"));
        } });
    mws.addHandler("/api/apps", HTTP_GET, []()
                   { mws.webserver->send_P(200, "application/json", DisplayManager.getAppsWithIcon().c_str()); });
    mws.addHandler("/api/settings", HTTP_POST, []()
                   { DisplayManager.setNewSettings(mws.webserver->arg("plain").c_str()); mws.webserver->send(200,F("text/plain"),F("OK")); });
    mws.addHandler("/api/erase", HTTP_ANY, []()
                   { LittleFS.format(); delay(200); formatSettings();   mws.webserver->send(200,F("text/plain"),F("OK"));delay(200); ESP.restart(); });
    mws.addHandler("/api/resetSettings", HTTP_ANY, []()
                   { formatSettings();   mws.webserver->send(200,F("text/plain"),F("OK"));delay(200); ESP.restart(); });
    mws.addHandler("/api/reorder", HTTP_POST, []()
                   { DisplayManager.reorderApps(mws.webserver->arg("plain").c_str()); mws.webserver->send(200,F("text/plain"),F("OK")); });
    mws.addHandler("/api/settings", HTTP_GET, []()
                   { mws.webserver->send_P(200, "application/json", DisplayManager.getSettings().c_str()); });
    mws.addHandler("/api/custom", HTTP_POST, []()
                   { 
                    if (DisplayManager.parseCustomPage(mws.webserver->arg("name"),mws.webserver->arg("plain").c_str(),false)){
                        mws.webserver->send(200,F("text/plain"),F("OK")); 
                    }else{
                        mws.webserver->send(500,F("text/plain"),F("ErrorParsingJson")); 
                    } });
    mws.addHandler("/api/stats", HTTP_GET, []()
                   { mws.webserver->send_P(200, "application/json", DisplayManager.getStats().c_str()); });
    mws.addHandler("/api/screen", HTTP_GET, []()
                   { mws.webserver->send_P(200, "application/json", DisplayManager.ledsAsJson().c_str()); });
    mws.addHandler("/api/indicator1", HTTP_POST, []()
                   { 
                    if (DisplayManager.indicatorParser(1,mws.webserver->arg("plain").c_str())){
                     mws.webserver->send(200,F("text/plain"),F("OK")); 
                    }else{
                         mws.webserver->send(500,F("text/plain"),F("ErrorParsingJson")); 
                    } });
    mws.addHandler("/api/indicator2", HTTP_POST, []()
                   { 
                    if (DisplayManager.indicatorParser(2,mws.webserver->arg("plain").c_str())){
                     mws.webserver->send(200,F("text/plain"),F("OK")); 
                    }else{
                         mws.webserver->send(500,F("text/plain"),F("ErrorParsingJson")); 
                    } });
    mws.addHandler("/api/indicator3", HTTP_POST, []()
                   { 
                    if (DisplayManager.indicatorParser(3,mws.webserver->arg("plain").c_str())){
                     mws.webserver->send(200,F("text/plain"),F("OK")); 
                    }else{
                         mws.webserver->send(500,F("text/plain"),F("ErrorParsingJson")); 
                    } });
    mws.addHandler("/api/r2d2", HTTP_POST, []()
                   { PeripheryManager.r2d2(mws.webserver->arg("plain").c_str()); mws.webserver->send(200,F("text/plain"),F("OK")); });

    mws.addHandler("/api/controller", HTTP_POST, []()
                   {
        GameManager.ControllerInput(mws.webserver->arg("key"), mws.webserver->arg("status"));
        mws.webserver->send(200, F("text/plain"), F("OK")); });
}

void ServerManager_::setup()
{
    if (!local_IP.fromString(NET_IP) || !gateway.fromString(NET_GW) || !subnet.fromString(NET_SN) || !primaryDNS.fromString(NET_PDNS) || !secondaryDNS.fromString(NET_SDNS))
        NET_STATIC = false;
    if (NET_STATIC)
    {
        WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS);
    }
    WiFi.setHostname(uniqueID); // define hostname
    myIP = mws.startWiFi(15000, uniqueID, "12345678");
    isConnected = !(myIP == IPAddress(192, 168, 4, 1));
    if (DEBUG_MODE)
        DEBUG_PRINTF("My IP: %d.%d.%d.%d", myIP[0], myIP[1], myIP[2], myIP[3]);
    mws.setAuth(AUTH_USER, AUTH_PASS);
    if (isConnected)
    {
        mws.addOptionBox("Network");
        mws.addOption("Static IP", NET_STATIC);
        mws.addOption("Local IP", NET_IP);
        mws.addOption("Gateway", NET_GW);
        mws.addOption("Subnet", NET_SN);
        mws.addOption("Primary DNS", NET_PDNS);
        mws.addOption("Secondary DNS", NET_SDNS);
        mws.addOptionBox("MQTT");
        mws.addOption("Broker", MQTT_HOST);
        mws.addOption("Port", MQTT_PORT);
        mws.addOption("Username", MQTT_USER);
        mws.addOption("Password", MQTT_PASS);
        mws.addOption("Prefix", MQTT_PREFIX);
        mws.addOption("Homeassistant Discovery", HA_DISCOVERY);
        mws.addOptionBox("Time");
        mws.addOption("NTP Server", NTP_SERVER);
        mws.addOption("Timezone", NTP_TZ);
        mws.addHTML("<p>Find your timezone at <a href='https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv' target='_blank' rel='noopener noreferrer'>posix_tz_db</a>.</p>", "tz_link");
        mws.addOptionBox("Icons");
        mws.addHTML(custom_html, "icon_html");
        mws.addCSS(custom_css);
        mws.addJavascript(custom_script);
        mws.addOptionBox("Auth");
        mws.addOption("Auth Username", AUTH_USER);
        mws.addOption("Auth Password", AUTH_PASS);
        mws.addHandler("/save", HTTP_POST, saveHandler);
        addHandler();
        udp.begin(localUdpPort);
        if (DEBUG_MODE)
            DEBUG_PRINTLN(F("Webserver loaded"));
    }
    mws.addHandler("/version", HTTP_GET, versionHandler);
    mws.begin();

    if (!MDNS.begin(MQTT_PREFIX))
    {
        if (DEBUG_MODE)
            DEBUG_PRINTLN(F("Error starting mDNS"));
    }
    else
    {
        MDNS.addService("http", "tcp", 80);
        MDNS.addService("awtrix", "tcp", 80);
        MDNS.addServiceTxt("awtrix", "tcp", "id", uniqueID);
        MDNS.addServiceTxt("awtrix", "tcp", "name", MQTT_PREFIX);
        MDNS.addServiceTxt("awtrix", "tcp", "type", "awtrix_light");
    }

    configTzTime(NTP_TZ.c_str(), NTP_SERVER.c_str());
    tm timeInfo;
    getLocalTime(&timeInfo);
}

void ServerManager_::tick()
{
    mws.run();

    if (!AP_MODE)
    {
        int packetSize = udp.parsePacket();
        if (packetSize)
        {
            int len = udp.read(incomingPacket, 255);
            if (len > 0)
            {
                incomingPacket[len] = 0;
            }
            if (strcmp(incomingPacket, "FIND_AWTRIX") == 0)
            {
                udp.beginPacket(udp.remoteIP(), 4211);
                udp.printf(MQTT_PREFIX.c_str());
                udp.endPacket();
            }
        }
    }
}

void ServerManager_::loadSettings()
{
    if (LittleFS.exists("/DoNotTouch.json"))
    {
        File file = LittleFS.open("/DoNotTouch.json", "r");
        DynamicJsonDocument doc(file.size() * 1.33);
        DeserializationError error = deserializeJson(doc, file);
        if (error)
            return;

        NTP_SERVER = doc["NTP Server"].as<String>();
        NTP_TZ = doc["Timezone"].as<String>();
        MQTT_HOST = doc["Broker"].as<String>();
        MQTT_PORT = doc["Port"].as<uint16_t>();
        MQTT_USER = doc["Username"].as<String>();
        MQTT_PASS = doc["Password"].as<String>();
        MQTT_PREFIX = doc["Prefix"].as<String>();
        NET_STATIC = doc["Static IP"];
        HA_DISCOVERY = doc["Homeassistant Discovery"];
        NET_IP = doc["Local IP"].as<String>();
        NET_GW = doc["Gateway"].as<String>();
        NET_SN = doc["Subnet"].as<String>();
        NET_PDNS = doc["Primary DNS"].as<String>();
        NET_SDNS = doc["Secondary DNS"].as<String>();
        if (doc["Auth Username"].is<String>())
            AUTH_USER = doc["Auth Username"].as<String>();
        if (doc["Auth Password"].is<String>())
            AUTH_PASS = doc["Auth Password"].as<String>();

        file.close();
        DisplayManager.applyAllSettings();
        if (DEBUG_MODE)
            DEBUG_PRINTLN(F("Webserver configuration loaded"));
        doc.clear();
        return;
    }
    else if (DEBUG_MODE)
        DEBUG_PRINTLN(F("Webserver configuration file not exist"));
    return;
}

void ServerManager_::sendButton(byte btn, bool state)
{
    if (BUTTON_CALLBACK == "")
        return;
    static bool btn0State, btn1State, btn2State;
    String payload;
    switch (btn)
    {
    case 0:
        if (btn0State != state)
        {
            btn0State = state;
            payload = "button=left&state=" + String(state);
        }
        break;
    case 1:
        if (btn1State != state)
        {
            btn1State = state;
            payload = "button=middle&state=" + String(state);
        }
        break;
    case 2:
        if (btn2State != state)
        {
            btn2State = state;
            payload = "button=right&state=" + String(state);
        }
        break;
    default:
        return;
    }
    if (!payload.isEmpty())
    {
        HTTPClient http;
        http.begin(BUTTON_CALLBACK);
        http.addHeader("Content-Type", "application/x-www-form-urlencoded");
        http.POST(payload);
        http.end();
    }
}
