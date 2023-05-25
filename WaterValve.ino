#include <esp-fs-webserver.h>  // https://github.com/cotestatnt/esp-fs-webserver

#include <FS.h>
#include <LittleFS.h>
#define FILESYSTEM LittleFS

#ifndef LED_BUILTIN
#define LED_BUILTIN 1
#endif

// In order to set SSID and password open the /setup webserver page
// const char* ssid;
// const char* password;

uint8_t ledPin = LED_BUILTIN;
uint8_t relayPin = 0;
bool apMode = false;

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
    while (file){
      const char* fileName = file.name();
      size_t fileSize = file.size();
      Serial.printf("FS File: %s, size: %lu\n", fileName, (long unsigned)fileSize);
      file = root.openNextFile();
    }
    Serial.println();
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
    //delay (2000);
    //Serial.print(value);
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
    //delay (2000);
    //Serial.print(value);
  }

  String reply;// = "Реле ";
  reply += digitalRead(relayPin) ? "Выкл" : "Вкл";
  webRequest->send(200, "text/plain", reply);
}

void handleTemp() {
  float temp1 = 36.6;
  float temp2 = 42.0;
  int value;
  String reply;
  WebServerClass* webRequest = myWebServer.getRequest();

  // http://xxx.xxx.xxx.xxx/led?val=1
  if(webRequest->hasArg("val")) {
    value = webRequest->arg("val").toInt();
    //digitalWrite(relayPin, value);
    //delay (2000);
    //Serial.print(value);
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
  Serial.begin(115200);
  digitalWrite(ledPin, 1);
  digitalWrite(relayPin, 1);
  // FILESYSTEM INIT
  startFilesystem();

  // Try to connect to flash stored SSID, start AP if fails after timeout
  IPAddress myIP = myWebServer.startWiFi(15000, "ESP8266_AP", "123456789" );

  // Add custom page handlers to webserver
  myWebServer.addHandler("/led", HTTP_GET, handleLed);
  myWebServer.addHandler("/relay", HTTP_GET, handleRelay);
  myWebServer.addHandler("/temp", HTTP_GET, handleTemp);
  // Start webserver
  if (myWebServer.begin()) {
    Serial.print(F("ESP Web Server started on IP Address: "));
    Serial.println(myIP);
    Serial.println(F("Open /setup page to configure optional parameters"));
    Serial.println(F("Open /edit page to view and edit files"));
    Serial.println(F("Open /update page to upload firmware and filesystem updates"));
  }

  pinMode(ledPin, OUTPUT);
  pinMode(relayPin, OUTPUT);

}


void loop() {
  myWebServer.run();

}
