#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <LittleFS.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// OLED display width and height
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_I2C_ADDRESS 0x3C // Update this to the correct I2C address

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET -1  // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const char* ssid = "testbench";
const char* password = "testbench";

ESP8266WebServer server(80);

void callExternalAPI(String prompt);

void setup() {
  Serial.begin(9600);
  Serial.println("Starting setup...");

  // Initialize the OLED display
  if(!display.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  display.clearDisplay();
  display.setTextSize(1);      // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE); // Draw white text
  display.setCursor(0, 0);     // Start at top-left corner
  display.println(F("Starting setup..."));
  display.display();

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected successfully");
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println(F("WiFi connected successfully"));
  display.display();

  if (!LittleFS.begin()) {
    Serial.println("Failed to mount LittleFS");
    display.println(F("Failed to mount LittleFS"));
    display.display();
    return;
  } else {
    Serial.println("Mounted LittleFS successfully");
    display.println(F("Mounted LittleFS successfully"));
    display.display();
  }

  server.on("/", HTTP_GET, []() {
    Serial.println("Handling / request");
    File file = LittleFS.open("/webpage/index.html", "r");
    if (!file) {
      Serial.println("Failed to open index.html");
    } else {
      Serial.println("Serving index.html");
      server.streamFile(file, "text/html");
      file.close();
    }
  });

  server.on("/css/style.css", HTTP_GET, []() {
    Serial.println("Handling /css/style.css request");
    File file = LittleFS.open("/webpage/css/style.css", "r");
    if (!file) {
      Serial.println("Failed to open style.css");
    } else {
      Serial.println("Serving style.css");
      server.send(200, "text/css", file.readString());
      file.close();
    }
  });

  server.on("/js/script.js", HTTP_GET, []() {
    Serial.println("Handling /js/script.js request");
    File file = LittleFS.open("/webpage/js/script.js", "r");
    if (!file) {
      Serial.println("Failed to open script.js");
    } else {
      Serial.println("Serving script.js");
      server.send(200, "application/javascript", file.readString());
      file.close();
    }
  });

  server.on("/api/query", HTTP_POST, []() {
    Serial.println("Received API query");
    if (server.hasArg("plain") == false) {
      Serial.println("Bad Request: No 'plain' argument");
      server.send(400, "text/plain", "Bad Request");
      return;
    }

    String prompt = server.arg("plain");
    Serial.println("Calling external API with prompt: " + prompt);
    callExternalAPI(prompt);
  });

  server.begin();
  Serial.println("HTTP server started");
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println(F("HTTP server started"));
  display.display();
}

void callExternalAPI(String prompt) {
  Serial.println("Attempting to call external API");
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println(F("Calling external API"));
  display.display();

  if(WiFi.status() == WL_CONNECTED){
    DynamicJsonDocument doc(1024); // Adjust size according to needs
    doc["model"] = "tinyllama";
    doc["prompt"] = prompt;
    doc["stream"] = false;

    String requestBody;
    serializeJson(doc, requestBody);

    HTTPClient http;
    WiFiClient client; // Create a WiFiClient instance
    http.begin(client, "http://192.168.237.246:11434/api/generate"); // Use the new begin method with WiFiClient
    http.addHeader("Content-Type", "application/json");

    Serial.println("Sending HTTP POST request");
    int httpResponseCode = http.POST(requestBody);

    if(httpResponseCode > 0) {
      String response = http.getString();
      Serial.println("Received response from API");
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println(F("Received response:"));
      display.println(response);
      display.display();
      server.send(200, "application/json", response);
    }
    else {
      Serial.printf("Failed to get a response, error code: %d\n", httpResponseCode);
      display.clearDisplay();
      display.setCursor(0, 0);
      display.printf("API Error: %d\n", httpResponseCode);
      display.display();
      server.send(500, "text/plain", "Server Error");
    }

    http.end();
  }
  else {
    Serial.println("WiFi not connected, cannot call API");
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println(F("WiFi not connected"));
    display.display();
    server.send(503, "text/plain", "Service Unavailable");
  }
}

void loop() {
  server.handleClient();

  // Check WiFi connection status
  if(WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi disconnected, attempting to reconnect...");
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println(F("WiFi disconnected"));
    display.display();
    WiFi.reconnect();
    while(WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    Serial.println("\nWiFi reconnected successfully.");
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println(F("WiFi reconnected"));
    display.display();
  }
}
