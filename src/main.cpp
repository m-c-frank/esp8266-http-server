#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <LittleFS.h>

const char* ssid = "testbench";
const char* password = "testbench";

ESP8266WebServer server(80);

void setup() {
  Serial.begin(9600);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  if (!LittleFS.begin()) {
    Serial.println("An error has occurred while mounting LittleFS");
    return;
  }

  server.on("/", HTTP_GET, []() {
    File file = LittleFS.open("/webpage/index.html", "r");
    server.streamFile(file, "text/html");
    file.close();
  });

  server.on("/css/style.css", HTTP_GET, []() {
    File file = LittleFS.open("/webpage/css/style.css", "r");
    server.send(200, "text/css", file.readString());
    file.close();
  });

  server.on("/js/script.js", HTTP_GET, []() {
    File file = LittleFS.open("/webpage/js/script.js", "r");
    server.send(200, "application/javascript", file.readString());
    file.close();
  });

  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();

  // Check WiFi connection
  if(WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi disconnected, attempting to reconnect...");
    WiFi.reconnect();
    while(WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    Serial.println("\nWiFi reconnected.");
  }
}