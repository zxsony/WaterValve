#include <esp-fs-webserver.h>  // https://github.com/cotestatnt/esp-fs-webserver

//#include <FS.h>
#include <LittleFS.h>
#include <Ds1820Adv.h>
#include <OneWire.h>
#include <Streaming.h>
#define FILESYSTEM LittleFS

#ifndef LED_BUILTIN
#define LED_BUILTIN 1
#endif

#define DEBUG true

const char* ssid = "NVRAM WARNING";
const char* pass = "Lift80Lift";
uint8_t retries = 0;
// For ESP-01: GPIO0, GPIO2 - free. GPIO1 - TX, GPIO3 - RX.
uint8_t ledPin = 2;
uint8_t relayPin = 0;
bool apMode = false;
OneWire oneWire(3);//3
Ds1820Adv ds(&oneWire);
float temp1 = 0;
float temp2 = 0;

#ifdef ESP8266
  ESP8266WebServer server(80);
#elif defined(ESP32)
  WebServer server(80);
#endif
FSWebServer myWebServer(FILESYSTEM, server);


////////////////////////////////  Filesystem  /////////////////////////////////////////
void startFilesystem(){
  // FILESYSTEM INIT
  if ( FILESYSTEM.begin()){
    File root = FILESYSTEM.open("/", "r");
    File file = root.openNextFile();
#if DEBUG
    Serial.println();
#endif
    while (file){
      const char* fileName = file.name();
      size_t fileSize = file.size();
#if DEBUG
      Serial.printf("FS File: %s, size: %lu\n", fileName, (long unsigned)fileSize);
#endif
      file = root.openNextFile();
    }
#if DEBUG
    Serial.println();
#endif
  }
  else {
    Serial.println("ERROR on mounting filesystem. It will be formmatted!");
    FILESYSTEM.format();
    ESP.restart();
  }
}

////////////////////////////  HTTP Request Handlers  ////////////////////////////////////
void handleLed() {
  WebServerClass* webRequest = myWebServer.getRequest();

  // http://xxx.xxx.xxx.xxx/led?val=1
  if(webRequest->hasArg("val")) {
    int value = webRequest->arg("val").toInt();
    digitalWrite(ledPin, value);
  }

  String reply;// = "Светодиод ";
  reply += digitalRead(ledPin) ? "Выкл" : "Вкл";
  webRequest->send(200, "text/plain", reply);
}

void handleRelay() {
  WebServerClass* webRequest = myWebServer.getRequest();

  // http://xxx.xxx.xxx.xxx/led?val=1
  if(webRequest->hasArg("val")) {
    int value = webRequest->arg("val").toInt();
    digitalWrite(relayPin, value);
  }

  String reply;// = "Реле ";
  reply += digitalRead(relayPin) ? "Выкл" : "Вкл";
  webRequest->send(200, "text/plain", reply);
}

void handleTemp() {
  int value;
  String reply;
  WebServerClass* webRequest = myWebServer.getRequest();

  // http://xxx.xxx.xxx.xxx/led?val=1
  if(webRequest->hasArg("val")) {
    value = webRequest->arg("val").toInt();
  }

  if(value == 1){
  reply += temp1;
  }
  if(value == 2){
  reply += temp2;
  }  
  webRequest->send(200, "text/plain", reply);
}

void setup(){
  Serial.begin(115200,SERIAL_8N1,SERIAL_TX_ONLY);
  ds.dsInit();
  digitalWrite(ledPin, 1);
  digitalWrite(relayPin, 1);
  // FILESYSTEM INIT
  startFilesystem();

  // Try to connect to flash stored SSID, start AP if fails after timeout
  //IPAddress myIP = myWebServer.startWiFi(15000, "ESP8266_AP", "123456789" );

  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP("zxSTA_APzx");
  WiFi.softAPConfig(IPAddress(192, 168, 4, 4), IPAddress(192, 168, 4, 4), IPAddress(255, 255, 255, 0));
  WiFi.begin(ssid, pass);
  while(WiFi.status()!= WL_CONNECTED && retries < 15){
#if DEBUG
    Serial.print(".");
#endif
    retries++;
    delay(1000);
  }
      if (WiFi.waitForConnectResult() == WL_CONNECTED) {
#if DEBUG
    Serial.print("Connected to ");
    Serial.println(ssid);
#endif
      }
  else {
    WiFi.mode(WIFI_AP);
//    WiFi.softAPConfig(IPAddress(192, 168, 4, 4), IPAddress(192, 168, 4, 4), IPAddress(255, 255, 255, 0));
    WiFi.softAP("zxAPzx");
#if DEBUG
    Serial.println("softAP");
#endif
//    WiFi.begin();
//    Serial.println(WiFi.softAPIP());
  }

  // Add custom page handlers to webserver
  myWebServer.addHandler("/led", HTTP_GET, handleLed);
  myWebServer.addHandler("/relay", HTTP_GET, handleRelay);
  myWebServer.addHandler("/temp", HTTP_GET, handleTemp);
  // Start webserver
  if (myWebServer.begin()) {
#if DEBUG
    Serial.println();
    Serial.print(F("ESP Web Server started on IP Address: "));
    Serial.println(WiFi.localIP()); //Serial.println(myIP);
    Serial.println(F("Open /setup page to configure optional parameters"));
    Serial.println(F("Open /edit page to view and edit files"));
    Serial.println(F("Open /update page to upload firmware and filesystem updates"));
#endif
  }

  pinMode(ledPin, OUTPUT);
  pinMode(relayPin, OUTPUT);
}

void loop() {
  myWebServer.run();
  if (ds.dsQuery()) {
#if DEBUG
    ds.dsPrintTemp();
#endif
    if (ds.dsCount > 1){
      temp1 = ds.dsTemp[0];
      temp2 = ds.dsTemp[1];
    }
    else if(ds.dsCount > 0) {
      temp1 = ds.dsTemp[0];
    }
  }
}
