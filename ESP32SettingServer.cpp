#include "ESP32SettingServer.h"

ESP32SettingServer::ESP32SettingServer(uint16_t port, uint8_t third_octet) : 
	_server(port), 
	_local_ip(192, 168, third_octet, 1),
	_gateway(192, 168, third_octet, 1),
	_subnet(255, 255, 255, 0),
	_doc(1)
{
	_esp_ssid = "ESP32-AP";
	_esp_pass = "12345678";
	_setting_fpath = "/settings.txt";
}

void ESP32SettingServer::setWiFiAccessPointInfo(String esp_ssid, String esp_pass) {
	_esp_ssid = esp_ssid;
	_esp_pass = esp_pass;
}

void ESP32SettingServer::setSettingFilePath(String fpath) {
	_setting_fpath = fpath;
}

void ESP32SettingServer::enableDebugPrint(bool enable) {
	_debug_enable = enable;
}

void ESP32SettingServer::begin(bool reset_settings) {
	_debugPrint("Initialize setting template...");
	setSettingTemplate(defineSettingTemplate());

	if (reset_settings || !loadSettings()) {
		_debugPrint("Initialized setting file");
		saveSettings();
		loadSettings();
	}
}

void ESP32SettingServer::serverStart() {
	_debugPrint("Prepare WiFiAP start");
	WiFi.softAP(_esp_ssid.c_str(), _esp_pass.c_str());
	delay(100);
	WiFi.softAPConfig(_local_ip, _gateway, _subnet); // setup ip, gateway, subnet_mask
    delay(100);
	_debugPrint("WiFiAP OK!");

	_server.on("/", HTTP_GET, [this](){handle_OnRootGet();});
	_server.on("/", HTTP_POST, [this](){handle_OnRootPost();});
	_server.onNotFound([this](){handle_NotFound();});

	_server.begin();
}

void ESP32SettingServer::loop() {
	_server.handleClient();
}

void ESP32SettingServer::loopForever() {
	while(1) {
		loop();
		delay(1);
	}
}

bool ESP32SettingServer::saveSettings(){
	if (!SPIFFS.begin(true)) {
		_debugPrint("Failed to begin SPIFFS.");
		return false;
	}
	
	String tmpl = "";
	serializeJson(_doc, tmpl);

	File f = SPIFFS.open(_setting_fpath.c_str(), "w");
	if (!f) {
		_debugPrint("Open error.");
		SPIFFS.end();
		return false;
	}
	f.print(tmpl);
	f.close();
	SPIFFS.end();
	return true;
}

bool ESP32SettingServer::loadSettings(){
	if (!SPIFFS.begin(true)) {
		_debugPrint("Failed to begin SPIFFS.");
		return false;
	}

	if (!SPIFFS.exists(_setting_fpath.c_str())) {
		_debugPrint("File not exists.");
		SPIFFS.end();
		return false;
	}
	
	File f = SPIFFS.open(_setting_fpath.c_str(), "r");
	if (!f) {
		_debugPrint("Open error.");
		SPIFFS.end();
		return false;
	}

	String json = "";
	for (unsigned int i=0; i<f.size(); i++) {
		json += (char)f.read();
	}
	f.close();
	deserializeJson(_doc, json.c_str());

	SPIFFS.end();
	return true;
}

DynamicJsonDocument ESP32SettingServer::getSettingsRaw() {
	return _doc;
}

DynamicJsonDocument ESP32SettingServer::defineSettingTemplate() {
	DynamicJsonDocument ddoc(1);
	return ddoc;
}

void ESP32SettingServer::setSettingTemplate(DynamicJsonDocument setting) {
	_doc = setting;
	_doc.garbageCollect();
}


/****************************************
 * Protected functions
****************************************/
void ESP32SettingServer::handle_OnRootGet() {
	String html = R"(<!DOCTYPE html>
	<html>
	<head>
		<meta name='viewport' content='width=device-width,initial-scale=1.0'>
		<meta charset='UTF-8'>
		<title> ESP32 Setting </title>
	</head>
	<body style='max-width: 600px; margin: auto;'>
		<h1 style='text-align: center;'>ESP32 Settings</h1>
		
		<form method='post' style="text-align: center;">)";
		
	JsonObject root = _doc.as<JsonObject>();
	for (auto kv : root) {
    	html += "<label>" + String(kv.key().c_str()) + "</label></br>";
		for (auto elem : _doc[kv.key().c_str()].as<JsonObject>()) {
			auto name = String(kv.key().c_str()) + "_" + String(elem.key().c_str());
			html += "<label>" + String(elem.key().c_str()) + ":&ensp;</label>";
			html += "<input type='text'";
			html += "name='" + name + "\' " ;
			html += "placeholder='" + name + "\' ";
			html += "value='" + String(elem.value().as<char*>()) + "\' ";
			html += "></br>";
			//Serial.println(root[kv.key().c_str()][elem.key().c_str()].as<char*>());
		}
		html += "</br>";
	}


	html += R"(<input type='submit'><br>
		</form>
	</body>
	</html>)";
    _server.send(200, "text/html", html);
}

void ESP32SettingServer::handle_OnRootPost(){
	JsonObject root = _doc.as<JsonObject>();
	for (auto kv : root) {
		//Serial.println(kv.key().c_str());
		for (auto elem : _doc[kv.key().c_str()].as<JsonObject>()) {
			//Serial.print("\t");
			//Serial.print(elem.key().c_str());
			//Serial.print(" -> ");
			//Serial.println(server.arg(elem.key().c_str()));
			auto name = String(kv.key().c_str()) + "_" + String(elem.key().c_str());
			_doc[kv.key().c_str()][elem.key().c_str()] = _server.arg(name);
		}
	}
	saveSettings();
	
    String html = R"(<!DOCTYPE html><html><head><meta name=\"viewport\" content=\"width=device-width,initial-scale=1.0\"><meta charset=\"UTF-8\"><title> WiFi Setting </title></head>
    <body style=\"max-width: 600px; margin: auto;\">
    <h1 style=\"text-align: center;\">ESP32 WiFi Settings</h1>
    <h3 style=\"text-align: center;\"> Complete! </h3>)";
    //html += ssid + "<br>";
    //html += pass + "<br>";
    html += "</body></html>";
	
    _server.send(200, "text/html", html);
	_doc.garbageCollect();
}

void ESP32SettingServer::handle_NotFound()
{
    _server.send(404, "text/plain", "Not found");
}

void ESP32SettingServer::_debugPrint(const char* fmt, ...) {
	if (_debug_enable == false) {
		return;
	}

	char s[256] = {'\0'};
	int n = 256;
	va_list arg;

    va_start(arg, fmt);
    vsnprintf(s, n, fmt, arg);
    va_end(arg);

	Serial.print("[Debug] ");
	Serial.println(s);
}