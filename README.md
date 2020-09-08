# ESP32SettingServer

<!-- # Short Description -->

A library for dynamically configuring the parameters of ESP32 through a LAN.

<!-- # Badges -->

[![Github top language](https://img.shields.io/github/languages/top/takkaO/ESP32SettingServer)](https://github.com/takkaO/ESP32SettingServer/)
[![Github license](https://img.shields.io/github/license/takkaO/ESP32SettingServer)](https://github.com/takkaO/ESP32SettingServer/)

# Installation

1. Clone this repository.
2. Copy into Arduino library directory  (ex. ```C:\User\user_name\Document\Arduino\libraries```).

# Minimal Example

```c++ 
#include "ESP32SettingServer.h"
#define BUTTON 25

// Create setting server instance
ESP32SettingServer ss;

void setup()
{
	Serial.begin(115200);
	pinMode(BUTTON, INPUT);
	
	// initialize
	ss.begin();
	if (digitalRead(BUTTON) == LOW) {
		// If button is press, create local server
		Serial.println("Server mode.");
		ss.serverStart();
		ss.loopForever();
		// 1. Please access WiFi -> SSID="ESP32-AP", PASS="12345678"
		// 2. Open your browser
		// 3. Access to 192.168.4.1
	}
	Serial.println("Client mode.");

	String ssid = ss.getParameter("WIFI", "SSID", ssid);
	String pass = ss.getParameter("WIFI", "PASS", pass);
	int p1 = ss.getParameter("TEST", "P1", p1);
	double p2 = ss.getParameter("TEST", "P2", p2);

	Serial.println("[WIFI]");
	Serial.print("\t[SSID]: ");
	Serial.println(ssid);
	Serial.print("\t[PASS]: ");
	Serial.println(pass);

	Serial.println("[TEST]");
	Serial.print("\t[P1]: ");
	Serial.println(p1);
	Serial.print("\t[P2]: ");
	Serial.println(p2);
}

void loop()
{
}

// Your settingparameter here!
DynamicJsonDocument ESP32SettingServer::defineSettingTemplate()
{
	// A sufficient amount of memory must be allocated.
	DynamicJsonDocument doc(512);

	JsonObject WIFI = doc.createNestedObject("WIFI");
	WIFI["SSID"] = "my_ssid";
	WIFI["PASS"] = "my_pass";

	JsonObject TEST = doc.createNestedObject("TEST");
	TEST["P1"] = "99";
	TEST["P2"] = "3.14";

	return doc;
}
```

# Contributors

- [takkaO](https://github.com/takkaO)

<!-- CREATED_BY_LEADYOU_README_GENERATOR -->