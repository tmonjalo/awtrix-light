#include "Globals.h"
#include "Preferences.h"
#include <WiFi.h>
#include <ArduinoJson.h>
#include <LittleFS.h>
#include "effects.h"

Preferences Settings;

char *getID()
{
    uint8_t mac[6];
    WiFi.macAddress(mac);
    char *macStr = new char[24];
    snprintf(macStr, 24, "awtrix_%02x%02x%02x", mac[3], mac[4], mac[5]);
    if (DEBUG_MODE)
        DEBUG_PRINTLN(F("Starting filesystem"));
    return macStr;
}

void startLittleFS()
{
    if (DEBUG_MODE)
        DEBUG_PRINTLN(F("Starting filesystem"));
    if (LittleFS.begin())
    {
        if (LittleFS.exists("/config.json"))
        {
            LittleFS.rename("/config.json", "/DoNotTouch.json");
        }

#ifdef ULANZI
        LittleFS.mkdir("/MELODIES");
#endif
        LittleFS.mkdir("/ICONS");
        LittleFS.mkdir("/PALETTES");
        LittleFS.mkdir("/CUSTOMAPPS");
        if (DEBUG_MODE)
            DEBUG_PRINTLN(F("Filesystem started"));
    }
    else
    {
        if (DEBUG_MODE)
            DEBUG_PRINTLN(F("Filesystem corrupt. Formatting..."));
        LittleFS.format();
        ESP.restart();
    }
}

void loadDevSettings()
{
    if (DEBUG_MODE)
        DEBUG_PRINTLN("Loading Devsettings");
    if (LittleFS.exists("/dev.json"))
    {
        File file = LittleFS.open("/dev.json", "r");
        DynamicJsonDocument doc(1024);
        DeserializationError error = deserializeJson(doc, file);
        if (error)
        {
            if (DEBUG_MODE)
                DEBUG_PRINTLN(F("Failed to read dev settings"));
            return;
        }

        if (DEBUG_MODE)
            DEBUG_PRINTF("%i dev settings found", doc.size());

        if (doc.containsKey("bootsound"))
        {
            BOOT_SOUND = doc["bootsound"].as<String>();
        }

        if (doc.containsKey("sensor_reading"))
        {
            SENSOR_READING = doc["sensor_reading"].as<bool>();
        }

        if (doc.containsKey("dfplayer"))
        {
            DFPLAYER_ACTIVE = doc["dfplayer"].as<bool>();
        }

        if (doc.containsKey("matrix"))
        {
            MATRIX_LAYOUT = doc["matrix"];
        }

        if (doc.containsKey("mirror_screen"))
        {
            MIRROR_DISPLAY = doc["mirror_screen"].as<bool>();
        }

        if (doc.containsKey("temp_offset"))
        {
            TEMP_OFFSET = doc["temp_offset"];
        }

        if (doc.containsKey("min_battery"))
        {
            MIN_BATTERY = doc["min_battery"];
        }

        if (doc.containsKey("max_battery"))
        {
            MAX_BATTERY = doc["max_battery"];
        }

        if (doc.containsKey("background_effect"))
        {
            BACKGROUND_EFFECT = getEffectIndex(doc["background_effect"].as<const char *>());
        }

        if (doc.containsKey("min_brightness"))
        {
            MIN_BRIGHTNESS = doc["min_brightness"];
        }

        if (doc.containsKey("max_brightness"))
        {
            MAX_BRIGHTNESS = doc["max_brightness"];
        }

        if (doc.containsKey("ldr_factor"))
        {
            LDR_FACTOR = doc["ldr_factor"].as<float>();
        }

        if (doc.containsKey("ldr_gamma"))
        {
            LDR_GAMMA = doc["ldr_gamma"].as<float>();
        }

        if (doc.containsKey("hum_offset"))
        {
            HUM_OFFSET = doc["hum_offset"];
        }

        if (doc.containsKey("ha_prefix"))
        {
            HA_PREFIX = doc["ha_prefix"].as<String>();
        }

        if (doc.containsKey("stats_interval"))
        {
            STATS_INTERVAL = doc["stats_interval"].as<long>();
        }

        if (doc.containsKey("temp_dec_places"))
        {
            TEMP_DECIMAL_PLACES = doc["temp_dec_places"].as<int>();
        }

        if (doc.containsKey("rotate_screen"))
        {
            ROTATE_SCREEN = doc["rotate_screen"].as<bool>();
        }

        if (doc.containsKey("debug_mode"))
        {
            DEBUG_MODE = doc["debug_mode"].as<bool>();
        }

        if (doc.containsKey("let_it_snow"))
        {
            SNOW = doc["let_it_snow"].as<bool>();
        }

        if (doc.containsKey("new_year"))
        {
            NEWYEAR = doc["new_year"].as<bool>();
        }

        if (doc.containsKey("button_callback"))
        {
            BUTTON_CALLBACK = doc["button_callback"].as<String>();
        }

        if (doc.containsKey("color_correction"))
        {
            auto correction = doc["color_correction"];
            if (correction.is<JsonArray>() && correction.size() == 3)
            {
                uint8_t r = correction[0];
                uint8_t g = correction[1];
                uint8_t b = correction[2];
                COLOR_CORRECTION.setRGB(r, g, b);
            }
        }

        if (doc.containsKey("color_temperature"))
        {
            auto temperature = doc["color_temperature"];
            if (temperature.is<JsonArray>() && temperature.size() == 3)
            {
                uint8_t r = temperature[0];
                uint8_t g = temperature[1];
                uint8_t b = temperature[2];
                COLOR_TEMPERATURE.setRGB(r, g, b);
            }
        }

        file.close();
    }
    else
    {
        if (DEBUG_MODE)
            DEBUG_PRINTLN("Devsettings not found");
    }
}

void formatSettings()
{
    Settings.begin("awtrix", false);
    Settings.clear();
    Settings.end();
}

void loadSettings()
{
    startLittleFS();
    if (DEBUG_MODE)
        DEBUG_PRINTLN(F("Loading Usersettings"));
    Settings.begin("awtrix", false);
    BRIGHTNESS = Settings.getUInt("BRI", 120);
    BRIGHTNESS_PERCENT = map(BRIGHTNESS, 0, 255, 0, 100);
    AUTO_BRIGHTNESS = Settings.getBool("ABRI", false);
    UPPERCASE_LETTERS = Settings.getBool("UPPER", true);
    TEXTCOLOR_888 = Settings.getUInt("TCOL", 0xFFFFFF);
    CALENDAR_HEADER_COLOR = Settings.getUInt("CHCOL", 0xFF0000);
    CALENDAR_TEXT_COLOR = Settings.getUInt("CTCOL", 0x000000);
    CALENDAR_BODY_COLOR = Settings.getUInt("CBCOL", 0xFFFFFF);
    TRANS_EFFECT = Settings.getUInt("TEFF", 1);
    TIME_MODE = Settings.getUInt("TMODE", 1);
    TIME_COLOR = Settings.getUInt("TIME_COL", 0);
    DATE_COLOR = Settings.getUInt("DATE_COL", 0);
    TEMP_COLOR = Settings.getUInt("TEMP_COL", 0);
    HUM_COLOR = Settings.getUInt("HUM_COL", 0);
#ifdef ULANZI
    BAT_COLOR = Settings.getUInt("BAT_COL", 0);
#endif
    WDC_ACTIVE = Settings.getUInt("WDCA", 0xFFFFFF);
    WDC_INACTIVE = Settings.getUInt("WDCI", 0x666666);
    AUTO_TRANSITION = Settings.getBool("ATRANS", true);
    SHOW_WEEKDAY = Settings.getBool("WD", true);
    TIME_PER_TRANSITION = Settings.getUInt("TSPEED", 400);
    TIME_PER_APP = Settings.getUInt("ATIME", 7000);
    TIME_FORMAT = Settings.getString("TFORMAT", "%H %M");
    DATE_FORMAT = Settings.getString("DFORMAT", "%d.%m.%y");
    START_ON_MONDAY = Settings.getBool("SOM", true);
    BLOCK_NAVIGATION = Settings.getBool("BLOCKN", false);
    IS_CELSIUS = Settings.getBool("CEL", true);
    SHOW_TIME = Settings.getBool("TIM", true);
    SHOW_DATE = Settings.getBool("DAT", false);
    SHOW_TEMP = Settings.getBool("TEMP", true);
    SHOW_HUM = Settings.getBool("HUM", true);
    MATRIX_LAYOUT = Settings.getUInt("MAT", 0);
    SCROLL_SPEED = Settings.getUInt("SSPEED", 100);
#ifdef ULANZI
    SHOW_BAT = Settings.getBool("BAT", true);
#endif
    SOUND_ACTIVE = Settings.getBool("SOUND", true);
    ALARM_SOUND = Settings.getUInt("ALSOUND", 0);
#ifndef ULANZI
    DFP_VOLUME = Settings.getUInt("VOL", 20);
#endif
    Settings.end();
    uniqueID = getID();
    MQTT_PREFIX = String(uniqueID);
    loadDevSettings();
}

void saveSettings()
{
    if (DEBUG_MODE)
        DEBUG_PRINTLN(F("Saving usersettings"));
    Settings.begin("awtrix", false);
    Settings.putUInt("CHCOL", CALENDAR_HEADER_COLOR);
    Settings.putUInt("CTCOL", CALENDAR_TEXT_COLOR);
    Settings.putUInt("CBCOL", CALENDAR_BODY_COLOR);
    Settings.putUInt("TEFF", TRANS_EFFECT);
    Settings.putUInt("BRI", BRIGHTNESS);
    Settings.putBool("WD", SHOW_WEEKDAY);
    Settings.putBool("ABRI", AUTO_BRIGHTNESS);
    Settings.putBool("BLOCKN", BLOCK_NAVIGATION);
    Settings.putBool("ATRANS", AUTO_TRANSITION);
    Settings.putBool("UPPER", UPPERCASE_LETTERS);
    Settings.putUInt("TCOL", TEXTCOLOR_888);
    Settings.putUInt("TMODE", TIME_MODE);
    Settings.putUInt("TIME_COL", TIME_COLOR);
    Settings.putUInt("DATE_COL", DATE_COLOR);
    Settings.putUInt("TEMP_COL", TEMP_COLOR);
    Settings.putUInt("HUM_COL", HUM_COLOR);
#ifdef ULANZI
    Settings.putUInt("BAT_COL", BAT_COLOR);
#endif
    Settings.putUInt("WDCA", WDC_ACTIVE);
    Settings.putUInt("WDCI", WDC_INACTIVE);
    Settings.putUInt("TSPEED", TIME_PER_TRANSITION);
    Settings.putUInt("ATIME", TIME_PER_APP);
    Settings.putString("TFORMAT", TIME_FORMAT);
    Settings.putString("DFORMAT", DATE_FORMAT);
    Settings.putBool("SOM", START_ON_MONDAY);
    Settings.putBool("CEL", IS_CELSIUS);
    Settings.putBool("TIM", SHOW_TIME);
    Settings.putBool("DAT", SHOW_DATE);
    Settings.putBool("TEMP", SHOW_TEMP);
    Settings.putBool("HUM", SHOW_HUM);
    Settings.putUInt("SSPEED", SCROLL_SPEED);
#ifdef ULANZI
    Settings.putBool("BAT", SHOW_BAT);
#endif
    Settings.putBool("SOUND", SOUND_ACTIVE);
    Settings.putUInt("ALSOUND", ALARM_SOUND);
#ifndef ULANZI
    Settings.putUInt("VOL", DFP_VOLUME);
#endif
    Settings.end();
}

const char *uniqueID;
IPAddress local_IP;
IPAddress gateway;
IPAddress subnet;
IPAddress primaryDNS;
IPAddress secondaryDNS;
const char *VERSION = "0.94";

String MQTT_HOST = "";
uint16_t MQTT_PORT = 1883;
String MQTT_USER;
String MQTT_PASS;
String MQTT_PREFIX;
bool IO_BROKER = false;
bool NET_STATIC = false;
bool SHOW_TIME = true;

bool SHOW_DATE = true;
bool SHOW_WEATHER = true;
bool SHOW_BAT = true;
bool SHOW_TEMP = true;
bool SHOW_HUM = true;
bool SHOW_SECONDS = true;
bool SHOW_WEEKDAY = true;
String NET_IP = "192.168.178.10";
String NET_GW = "192.168.178.1";
String NET_SN = "255.255.255.0";
String NET_PDNS = "8.8.8.8";
String NET_SDNS = "1.1.1.1";
long TIME_PER_APP = 7000;
uint8_t MATRIX_FPS = 42;
int TIME_PER_TRANSITION = 400;
String NTP_SERVER = "de.pool.ntp.org";
String NTP_TZ = "CET-1CEST,M3.5.0,M10.5.0/3";
bool HA_DISCOVERY = false;
String HA_PREFIX = "homeassistant";
// Periphery
String CURRENT_APP;
float CURRENT_TEMP;
bool IS_CELSIUS;
#ifndef ULANZI
uint8_t TEMP_SENSOR_TYPE = TEMP_SENSOR_TYPE_NONE;
#endif
float CURRENT_HUM;
float CURRENT_LUX;
int BRIGHTNESS = 120;
int BRIGHTNESS_PERCENT;

uint16_t MIN_BATTERY = 475;
uint16_t MAX_BATTERY = 665;

#ifdef awtrix2_upgrade
float TEMP_OFFSET;
#else
float TEMP_OFFSET = -9;
uint8_t BATTERY_PERCENT = 0;
uint16_t BATTERY_RAW = 0;
#endif
float HUM_OFFSET;
uint16_t LDR_RAW;
String TIME_FORMAT = "%H:%M:%S";
String DATE_FORMAT = "%d.%m.%y";
int BACKGROUND_EFFECT = -1;
bool START_ON_MONDAY;

// Matrix States
bool AUTO_TRANSITION = false;
bool AUTO_BRIGHTNESS = true;
bool UPPERCASE_LETTERS = true;
bool AP_MODE;
bool MATRIX_OFF;
bool MIRROR_DISPLAY = false;
uint32_t TEXTCOLOR_888 = 0xFFFFFF;
bool SOUND_ACTIVE;
String BOOT_SOUND = "";
uint8_t ALARM_SOUND = 0;
int TEMP_DECIMAL_PLACES = 0;
#ifndef ULANZI
uint8_t DFP_VOLUME;
#endif
int MATRIX_LAYOUT = 0;
long RECEIVED_MESSAGES;
CRGB COLOR_CORRECTION;
CRGB COLOR_TEMPERATURE;
uint32_t WDC_ACTIVE;
uint32_t WDC_INACTIVE;
bool BLOCK_NAVIGATION = false;
float GAMMA = 0;
bool SENSOR_READING = true;
bool SENSORS_STABLE = false;
bool DFPLAYER_ACTIVE = false;
bool ROTATE_SCREEN = false;
uint8_t TIME_MODE = 1;
uint8_t SCROLL_SPEED = 100;
uint32_t TIME_COLOR = 0;
uint32_t CALENDAR_HEADER_COLOR = 0xFF0000;
uint32_t CALENDAR_TEXT_COLOR = 0x000000;
uint32_t CALENDAR_BODY_COLOR = 0xFFFFFF;
uint32_t DATE_COLOR = 0;
uint32_t BAT_COLOR = 0;
uint32_t TEMP_COLOR = 0;
uint32_t HUM_COLOR = 0;
bool ARTNET_MODE;
bool MOODLIGHT_MODE;
long STATS_INTERVAL = 10000;
bool DEBUG_MODE = false;
uint8_t MIN_BRIGHTNESS = 2;
uint8_t MAX_BRIGHTNESS = 160;
double movementFactor = 0.5;
int8_t TRANS_EFFECT = 1;
String AUTH_USER = "";
String AUTH_PASS = "awtrix";
String BUTTON_CALLBACK = "";
bool SNOW = false;
bool NEWYEAR = false;
float LDR_GAMMA = 3.0;
float LDR_FACTOR = 1.0;
bool GAME_ACTIVE = false;

const char *MELODIES[MELODIES_MAX] PROGMEM = {
    "",
    "SIMPSONS:d=4,o=5,b=160:c.6,e6,f#6,8a6,g.6,e6,c6,8a,8f#,8f#,8f#,2g,8p,8p,8f#,8f#,8f#,8g,a#.,8c6,8c6,8c6,c6",
    "INDIANA:d=4,o=5,b=250:e,8p,8f,8g,8p,1c6,8p.,d,8p,8e,1f,p.,g,8p,8a,8b,8p,1f6,p,a,8p,8b,2c6,2d6,2e6,e,8p,8f,8g,8p,1c6,p,d6,8p,8e6,1f.6,g,8p,8g,e.6,8p,d6,8p,8g,e.6,8p,d6,8p,8g,f.6,8p,e6,8p,8d6,2c6",
    "LOONEY:d=4,o=5,b=140:32p,c6,8f6,8e6,8d6,8c6,a.,8c6,8f6,8e6,8d6,8d#6,e.6,8e6,8e6,8c6,8d6,8c6,8e6,8c6,8d6,8a,8c6,8g,8a#,8a,8f",
    "20th Fox:d=16,o=5,b=140:b,8p,b,b,2b,p,c6,32p,b,32p,c6,32p,b,32p,c6,32p,b,8p,b,b,b,32p,b,32p,b,32p,b,32p,b,32p,b,32p,b,32p,g#,32p,a,32p,b,8p,b,b,2b,4p,8e,8g#,8b,1c#6,8f#,8a,8c#6,1e6,8a,8c#6,8e6,1e6,8b,8g#,8a,2b",
    "STARWARS:d=4,o=5,b=45:32p,32f#,32f#,32f#,8b.,8f#.6,32e6,32d#6,32c#6,8b.6,16f#.6,32e6,32d#6,32c#6,8b.6,16f#.6,32e6,32d#6,32e6,8c#.6,32f#,32f#,32f#,8b.,8f#.6,32e6,32d#6,32c#6,8b.6,16f#.6,32e6,32d#6,32c#6,8b.6,16f#.6,32e6,32d#6,32e6,8c#6",
    "TOP GUN:d=4,o=4,b=31:32p,16c#,16g#,16g#,32f#,32f,32f#,32f,16d#,16d#,32c#,32d#,16f,32d#,32f,16f#,32f,32c#,16f,d#,16c#,16g#,16g#,32f#,32f,32f#,32f,16d#,16d#,32c#,32d#,16f,32d#,32f,16f#,32f,32c#,g#",
    "SCHTROU:d=32,o=5,b=200:4c#6,16p,4f#6,p,16c#6,p,8d#6,p,8b,p,4g#,16p,4c#6,p,16a#,p,8f#,p,8a#,p,4g#,4p,g#,p,a#,p,b,p,c6,p,4c#6,16p,4f#6,p,16c#6,p,8d#6,p,8b,p,4g#,16p,4c#6,p,16a#,p,8b,p,8f,p,4f#",
    "ENTERTAI:d=4,o=5,b=140:8d,8d#,8e,c6,8e,c6,8e,2c.6,8c6,8d6,8d#6,8e6,8c6,8d6,e6,8b,d6,2c6,p,8d,8d#,8e,c6,8e,c6,8e,2c.6,8p,8a,8g,8f#,8a,8c6,e6,8d6,8c6,8a,2d6",
    "GADGET:d=16,o=5,b=50:32d#,32f,32f#,32g#,a#,f#,a,f,g#,f#,32d#,32f,32f#,32g#,a#,d#6,4d6,32d#,32f,32f#,32g#,a#,f#,a,f,g#,f#,8d#",
    "MISSION:d=16,o=6,b=95:32d,32d#,32d,32d#,32d,32d#,32d,32d#,32d,32d,32d#,32e,32f,32f#,32g,g,8p,g,8p,a#,p,c7,p,g,8p,g,8p,f,p,f#,p,g,8p,g,8p,a#,p,c7,p,g,8p,g,8p,f,p,f#,p,a#,g,2d,32p,a#,g,2c#,32p,a#,g,2c,a#5,8c,2p,32p,a#5,g5,2f#,32p,a#5,g5,2f,32p,a#5,g5,2e,d#,8d",
};
