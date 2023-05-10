
/*    ___           ___                       ___                       ___
     /  /\         /__/\          ___        /  /\        ___          /__/|
    /  /::\       _\_ \:\        /  /\      /  /::\      /  /\        |  |:|
   /  /:/\:\     /__/\ \:\      /  /:/     /  /:/\:\    /  /:/        |  |:|
  /  /:/~/::\   _\_ \:\ \:\    /  /:/     /  /:/~/:/   /__/::\      __|__|:|
 /__/:/ /:/\:\ /__/\ \:\ \:\  /  /::\    /__/:/ /:/___ \__\/\:\__  /__/::::\____
 \  \:\/:/__\/ \  \:\ \:\/:/ /__/:/\:\   \  \:\/:::::/    \  \:\/\    ~\~~\::::/
  \  \::/       \  \:\ \::/  \__\/  \:\   \  \::/~~~~      \__\::/     |~~|:|~~
   \  \:\        \  \:\/:/        \  \:\   \  \:\          /__/:/      |  |:|
    \  \:\        \  \::/          \__\/    \  \:\         \__\/       |  |:|
     \__\/         \__\/                     \__\/                     |__|/

 ***************************************************************************
 *                                                                         *
 *   AWTRIX Light, a custom firmware for the Ulanzi clock                  *
 *                                                                         *
 *   Copyright (C) 2023  Stephan Mühl aka Blueforcer                       *
 *                                                                         *
 *   This work is licensed under a                                         *
 *   Creative Commons Attribution-NonCommercial-ShareAlike                 *
 *   4.0 International License.                                            *
 *                                                                         *
 *   More information:                                                     *
 *   https://github.com/Blueforcer/awtrix-light/blob/main/LICENSE.md       *
 *                                                                         *
 *   This firmware is distributed in the hope that it will be useful,      *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                  *
 *                                                                         *
 ***************************************************************************/

#include <Arduino.h>
#include "DisplayManager.h"
#include "PeripheryManager.h"
#include "MQTTManager.h"
#include "ServerManager.h"
#include "Globals.h"

void setup()
{
  pinMode(15, OUTPUT);
  digitalWrite(15, LOW);
  delay(2000);
  Serial.begin(115200);
  loadSettings();
  PeripheryManager.setup();
  ServerManager.loadSettings();
  DisplayManager.setup();
  DisplayManager.HSVtext(5, 6, "ULANZI", true, 0);
  delay(500);
  ServerManager.setup();
  if (ServerManager.isConnected)
  {
    DisplayManager.loadNativeApps();
    DisplayManager.loadCustomApps();
    DisplayManager.startArtnet();
    if (MQTT_HOST != "")
    {
      DisplayManager.HSVtext(4, 6, "MQTT...", true, 0);
      MQTTManager.setup();
      MQTTManager.tick();
    }
  }
  else
  {
    AP_MODE = true;
  }
  delay(200);
  DisplayManager.setBrightness(BRIGHTNESS);
}

void loop()
{
  ServerManager.tick();
  DisplayManager.tick();
  PeripheryManager.tick();
  if (ServerManager.isConnected)
  {
    MQTTManager.tick();
  }
}
