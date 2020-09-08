#ifndef ESP32_SETTING_SERVER_H
#define ESP32_SETTING_SERVER_H

#include "Arduino.h"
#include "WiFi.h"
#include "WebServer.h"
#include "SPIFFS.h"
#include "ArduinoJson.h"
#include <stdio.h>
#include <stdarg.h>

class ESP32SettingServer {
public:
	ESP32SettingServer(uint16_t port = 80, uint8_t third_octet = 4);
	void setWiFiAccessPointInfo(String esp_ssid, String esp_pass);
	void setSettingFilePath(String fpath);
	void enableDebugPrint(bool enable);
	void begin(bool reset_settings = false);
	void serverStart();
	void loop();
	void loopForever();
	bool saveSettings();
	bool loadSettings();
	DynamicJsonDocument getSettingsRaw();

	template <typename T>
	T getParameter(const char* category, const char* key, T target) {
		return _doc[category][key].as<T>();
	};

	void setSettingTemplate(DynamicJsonDocument setting);
	DynamicJsonDocument defineSettingTemplate() __attribute__((weak));

protected:
	void handle_OnRootGet();
	void handle_OnRootPost();
	void handle_NotFound();
	void _debugPrint(const char* fmt, ...);
	bool _debug_enable = true;

	String _esp_ssid;
	String _esp_pass;
	String _setting_fpath;

	WebServer _server;
	IPAddress _local_ip;
	IPAddress _gateway;
	IPAddress _subnet;
	DynamicJsonDocument _doc;
};

#endif
